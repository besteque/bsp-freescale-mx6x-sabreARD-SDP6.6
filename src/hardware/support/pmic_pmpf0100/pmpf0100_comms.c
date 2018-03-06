/*
 * $QNXLicenseC:
 * Copyright 2011, QNX Software Systems.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not reproduce, modify or distribute this software except in
 * compliance with the License. You may obtain a copy of the License
 * at: http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied.
 *
 * This file may contain contributions from others, either as
 * contributors under the License or as licensors under other terms.
 * Please review this entire file for other proprietary rights or license
 * notices, as well as the QNX Development Suite License Guide at
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */


#include <time.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <devctl.h>
#include <assert.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <arm/mx6x.h>
#include <hw/hwinfo_imx6x.h>
#include <hw/inout.h>
#include <hw/i2c.h>
#include "pmpf0100_comms.h"


#define PMPF0100_SEND_BUFFER_MAX_SIZE     32
#define PMPF0100_RECV_BUFFER_MAX_SIZE     32
#define PMPF0100_MAX_CALLBACKS            32

#define PMPF0100_PROCESS_PRIORITY         (15)

typedef struct
{
	pmic_pulse_info_t  pulse;
	int                connectionId;

} pmpf0100_event_info_t;


// General PMIC comms management structure
struct pmpf0100_comms_t
{
	int                    verbosity;        // Logging verbosity
	int                    fd;
	unsigned int           bus_number;
	unsigned int           bus_speed;
	unsigned int           bus_address;
	unsigned int           gpio_vbase;
	unsigned int           gpio_vbase_len;
	int                    irq_number;
	int                    irq_id;
    struct sigevent        irq_event;

    // I2C Message structures
    struct
	{
		i2c_send_t         hdr;
		uint8_t            data[PMPF0100_SEND_BUFFER_MAX_SIZE + 2];
	} send_msg;
    struct
	{
		i2c_sendrecv_t     hdr;
		uint8_t            data[PMPF0100_SEND_BUFFER_MAX_SIZE + 2];
	} sendrecv_msg;

	// Event Pulse Management
	struct
	{
		unsigned int           listInUse;
		pmpf0100_event_info_t   list[PMPF0100_MAX_CALLBACKS];
		pthread_mutex_t        mutex;
		uint32_t               nextGuid;
	} event;
};


uint16_t pmpf0100_irq_array[PMPF0100_NUM_IRQ_REGS] =
{
    PMPF0100_INT_STATUS0,
    PMPF0100_INT_STATUS1,
    PMPF0100_INT_STATUS3,
    PMPF0100_INT_STATUS4,
};

// Declaration of local functions
static int pmpf0100_comms_i2c_write(PMPF0100_COMMS *pComms, int incStop, uint32_t *data, size_t num);
static int pmpf0100_comms_i2c_write_read(PMPF0100_COMMS *pComms, int incStop, uint32_t write_data,
                                        uint32_t *read_data);

static int pmpf0100_comms_get_status(PMPF0100_COMMS *pComms, uint32_t baseReg, uint32_t *status);
static int pmpf0100_comms_set_status(PMPF0100_COMMS *pComms, uint32_t baseReg, uint32_t status);

int pmpf0100_comms_irq_event_process(PMPF0100_COMMS *pComms);


/*
 * Create a new PMIC comms object ready for use
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_open(unsigned i2c_bus, unsigned int i2c_address, PMPF0100_COMMS **ppComms)
{
	int result = EOK;
	char device_name[32];
	PMPF0100_COMMS *pComms;

	if (ppComms == NULL)
	{
		return EINVAL;
	}
	pComms = malloc(sizeof(PMPF0100_COMMS));
	if (pComms == NULL)
	{
		return ENOMEM;
	}

	pComms->bus_number = i2c_bus;
	pComms->bus_address = i2c_address;
	pComms->bus_speed = 0;
	pComms->verbosity = 0;

	sprintf(device_name, "/dev/i2c%d", pComms->bus_number);
	pComms->fd = open(device_name, O_RDWR);
	if (pComms->fd == -1)
	{
		free(pComms);
		return errno;
	}

	pComms->irq_number = -1;
	pComms->irq_id = -1;

	pComms->gpio_vbase = MAP_DEVICE_FAILED;
	pComms->gpio_vbase_len = 0;

	pComms->event.listInUse = 0;
	memset(pComms->event.list, 0, sizeof(pComms->event.list));
	pthread_mutex_init(&pComms->event.mutex, NULL);
	pComms->event.nextGuid = 1;

	// Mask all event interrupts
	result = pmpf0100_comms_set_status(pComms, PMPF0100_INT_MASK0, 0xFFFF);
	if (result == EOK)
	{
		// Clear all outstanding events
		result = pmpf0100_comms_set_status(pComms, PMPF0100_INT_STATUS0, 0);
	}

	*ppComms = pComms;
	return EOK;
}

/*
 * Close/Destroy an opened PMIC comms object
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_close(PMPF0100_COMMS **ppComms)
{
	PMPF0100_COMMS *pComms;

	if (ppComms == NULL)
	{
		return EINVAL;
	}
	if ((*ppComms) == NULL)
	{
		return EINVAL;
	}
	pComms = *ppComms;

	pmpf0100_comms_irq_uninit(pComms);

	pComms->event.listInUse = 0;

	if (pComms->fd != -1)
	{
		close(pComms->fd);
		pComms->fd = -1;
	}

	pthread_mutex_destroy(&pComms->event.mutex);

	free(pComms);

	(*ppComms) = NULL;

	return EOK;
}

/*
 * Get a lock on the PMIC comms
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_lock(PMPF0100_COMMS *pComms)
{
	if ((pComms == NULL) || (pComms->fd == -1))
	{
		return EINVAL;
	}
	return devctl(pComms->fd, DCMD_I2C_LOCK, NULL, 0, NULL);
}

/*
 * Release the lock on the PMIC comms
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_unlock(PMPF0100_COMMS *pComms)
{
	if ((pComms == NULL) || (pComms->fd == -1))
	{
		return EINVAL;
	}
	return devctl(pComms->fd, DCMD_I2C_UNLOCK, NULL, 0, NULL);
}

/*
 * Get the default host IRQ number for the PMIC
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_get_default_irq(PMPF0100_COMMS *pComms, int *irqNumber)
{
	int result = EOK;

	if ((pComms == NULL) || (irqNumber == NULL))
	{
		return EINVAL;
	}

	// should read the QNX syspage entry to get this info
	*irqNumber = QNX_GPIO4_16_IRQ;

	return result;
}

/*
 * Set the module logging verbosity
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_set_verbosity(PMPF0100_COMMS *pComms, int verbosity)
{
	if (pComms == NULL)
	{
		return EINVAL;
	}
	pComms->verbosity = verbosity;

	return EOK;
}

/*
 * Set the PMIC I2C comms speed
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_set_bus_speed(PMPF0100_COMMS *pComms, unsigned int bus_speed)
{
	int result;
	uint32_t speed;

	if ((pComms == NULL) || (pComms->fd == -1))
	{
		return EINVAL;
	}
	speed = bus_speed;
	result = devctl(pComms->fd, DCMD_I2C_SET_BUS_SPEED, &speed, sizeof(speed), NULL);
	if (result == EOK)
	{
		pComms->bus_speed = bus_speed;
	}

	return result;
}

/*
 * Get the current PMIC event status
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_read_status(PMPF0100_COMMS *pComms, uint32_t *regValue)
{
	return pmpf0100_comms_get_status(pComms, PMPF0100_INT_STATUS0, regValue);
}


/*
 * Read the contents of the specified PMIC register
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_read_register(PMPF0100_COMMS *pComms, uint32_t regNum, uint32_t *regValue)
{
	int  result;

	if ((pComms == NULL) || (pComms->fd == -1))
	{
		return EINVAL;
	}

	result = pmpf0100_comms_i2c_write_read(pComms, 1, regNum, regValue);

	if (result != EOK)
	{
    	slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
    	      "pmpf0100_comms_read_register i2c write read failed (code %d)", result);
	}

	return result;
}

/*
 * Write value to the specified PMIC register(s)
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_write_register(PMPF0100_COMMS *pComms, uint32_t regNum, uint32_t regValue)
{
	int      result = EOK;
	uint32_t msg[4];

	if ((pComms == NULL) || (pComms->fd == -1))
	{
		return EINVAL;
	}

    msg[0] = regNum;
    msg[1] = regValue;
    
    result = pmpf0100_comms_i2c_write(pComms, 1, msg, 2);
    if (result != EOK)
    {
        slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
              "pmpf0100_comms_write_register i2c write failed (code %d)", result);
	}

	return result;
}

/*
 * Register a pulse for a particular event
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_event_register(PMPF0100_COMMS *pComms, pmic_pulse_info_t *pCallback)
{
	int          result = EOK;
	unsigned int index;
	uint32_t     irqMask;
	pmpf0100_event_info_t *pListEntry;

	if ((pComms == NULL) || (pCallback == NULL))
	{
		return EINVAL;
	}
	if (pComms->irq_id == -1)
	{
		// IRQ handling not available
		return EINVAL;
	}
	if ((pCallback->eventMask == 0) || (pCallback->eventReg >= PMPF0100_NUM_IRQ_REGS)) 
	{
		// We must have something to check
		return EINVAL;
	}

	// Lock access to the callback list
	pthread_mutex_lock(&pComms->event.mutex);

	// Add the new entry to the end of the list
	index = pComms->event.listInUse;
	if (index >= PMPF0100_MAX_CALLBACKS)
	{
		// The list is full
		result = ENOMEM;
	}
	else
	{
		// Enable the appropriate PMIC interrupts (clear mask bits)
		pmpf0100_comms_lock(pComms);
		result = pmpf0100_comms_get_status(pComms,
                                           pmpf0100_irq_array[pCallback->eventReg],
                                           &irqMask);
		if (result == EOK)
		{
			irqMask &=  (pCallback->eventMask);
			result = pmpf0100_comms_set_status(pComms,
                                               pmpf0100_irq_array[pCallback->eventReg],
                                               irqMask);
		}
		pmpf0100_comms_unlock(pComms);

		if (result == EOK)
		{
			pComms->event.listInUse++;

			pListEntry = &pComms->event.list[index];

			// Store the settings
			pListEntry->pulse = *pCallback;

			// Create the pulse connection ID
			pListEntry->connectionId = ConnectAttach(0, pListEntry->pulse.ownerId,
                                                     pListEntry->pulse.channelId, _NTO_SIDE_CHANNEL, 0);
			if (pListEntry->connectionId == -1)
			{
				result = errno;
				slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR, "ConnectAttach failed (code %d)", result);
			}

			// Set & pass the new guid back (needed later to validate cancel)
			pListEntry->pulse.guid = pComms->event.nextGuid++;
			pCallback->guid = pListEntry->pulse.guid;

			if (pComms->verbosity > 4)
			{
				slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
						"pmpf0100_comms_event_register  index=%d, mask=0x%08X, pid=%d, chid=%d, coid=%d, pri=%d, code=%d, GUID=%d",
						index, pListEntry->pulse.eventMask,
						pListEntry->pulse.ownerId,
						pListEntry->pulse.channelId,
						pListEntry->connectionId,
						pListEntry->pulse.priority,
						pListEntry->pulse.code,
						pListEntry->pulse.guid);
			}

		}
	}

	// Unlock access to the callback list
	pthread_mutex_unlock(&pComms->event.mutex);

	return result;
}

/*
 * Cancel the registered pulse for the specified event
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_event_cancel(PMPF0100_COMMS *pComms, uint32_t eventGuid)
{
	int          result = EINVAL;
	int          index;
	uint32_t     cancelIrqMask;
	uint32_t     globalIrqMask;
	unsigned int numAfter;
	pmpf0100_event_info_t *pListEntry;

	if ((pComms == NULL) || (eventGuid == 0))
	{
		return EINVAL;
	}

	// Lock access to the callback list
	pthread_mutex_lock(&pComms->event.mutex);

	if (eventGuid < pComms->event.nextGuid)
	{
		result = ENOMSG;
		index = 0;
		while (index < pComms->event.listInUse)
		{
			pListEntry = &pComms->event.list[index];
			if (pListEntry->pulse.guid == eventGuid)
			{
				// Store the event mask;
				cancelIrqMask = pListEntry->pulse.eventMask;

				ConnectDetach(pListEntry->connectionId);
				pListEntry->connectionId = -1;

				// Remove the entry from the list
				numAfter = pComms->event.listInUse - (index + 1);
				if (numAfter > 0)
				{
					memmove(&pComms->event.list[index],
							&pComms->event.list[index+1],
							sizeof(pmpf0100_event_info_t) * numAfter);
				}
				pComms->event.listInUse--;

				// Work out what PMIC IRQ mask bits need to be set
				globalIrqMask = 0;
				for (index = 0; index < pComms->event.listInUse; index++)
				{
					globalIrqMask |= pComms->event.list[index].pulse.eventMask;
				}
				cancelIrqMask ^= globalIrqMask;
				cancelIrqMask &= ~(globalIrqMask);

				// Set the appropriate PMIC IRQ mask bits
				pmpf0100_comms_lock(pComms);
				result = pmpf0100_comms_get_status(pComms, PMPF0100_INT_MASK0, &globalIrqMask);
				if (result == EOK)
				{
					globalIrqMask |= cancelIrqMask;
					result = pmpf0100_comms_set_status(pComms, PMPF0100_INT_MASK0, globalIrqMask);
				}
				pmpf0100_comms_unlock(pComms);
				break;
			}
			index++;
		}
	}

	// Unlock access to the callback list
	pthread_mutex_unlock(&pComms->event.mutex);

	return result;
}


/*
 * Sets up hardware so that PMIC IRQ's can occur.
 * Then prepares all the IRQ servicing code.
 * Returns EOK if no problems, other code if problem
 *
 * The PMIC nIRQ line comes in on DI0_DISP_CLK and has a 10K pull up resistor,
 *  therefore a zero on that line indicates an IRQ.
 * We will route DI0_DISP_CLK to GPIO4[16] as it offers the best flexibility.
 * GPIO4[16] will then raise ARM interrupt 256 (GPIO4 bit 16).
 * That in turn will raise the QNX IRQ vector defined in 'init_intrinfo.c'.
 */
int pmpf0100_comms_irq_init(PMPF0100_COMMS *pComms, int *irqNumber, PMPF0100_IRQ_MODE irqMode)
{
	int result = EOK;

	if (pComms->irq_id != -1)
	{
		// Already setup
		if (pComms->verbosity > 3)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
		          "pmpf0100_comms_irq_init re-using existing irq setup");
		}
		return EOK;
	}

	if (irqNumber != NULL)
	{
		pComms->irq_number = *irqNumber;
	}
	else
	{
		pmpf0100_comms_get_default_irq(pComms, &pComms->irq_number);
	}

	// Give myself IO access
    ThreadCtl(_NTO_TCTL_IO, NULL);

	// Register for the interrupt
	memset(&pComms->irq_event, 0, sizeof(pComms->irq_event));
	SIGEV_INTR_INIT(&pComms->irq_event);
	pComms->irq_id = InterruptAttachEvent(pComms->irq_number, &pComms->irq_event, 0);
	if (pComms->irq_id == -1)
	{
		result = errno;
		slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
			  "pmpf0100_comms_irq_init failed InterruptAttachEvent (code %d)", result);
	}
	else
	{
		if (pComms->verbosity > 4)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
					"pmpf0100_comms_irq_init attached to IRQ %d", pComms->irq_number);
		}
	}

	// Tidy up if problem
	if (result != EOK)
	{
		pmpf0100_comms_irq_uninit(pComms);
	}

	return result;
}

/*
 * Prevents PMIC IRQ's occurring.
 * Also cleans up after error in pmpf0100_comms_irq_init.
 */
int pmpf0100_comms_irq_uninit(PMPF0100_COMMS *pComms)
{
	int       result = EOK;

	if (pComms->verbosity > 3)
	{
		slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
	          "pmpf0100_comms_irq_uninit entry");
	}

	// Detach the ISR
	if (pComms->irq_id != -1)
	{
		InterruptDetach(pComms->irq_id);
		pComms->irq_id = -1;
	}

	return result;
}


/*
 * Wait for an interrupt and then process the PMIC event flags
 * If nothing happens for timeout seconds, return.
 * Result = EOK = Interrupt processed correctly.
 *          ETIMEDOUT = No event for specified period.
 *          ENOTSUP = Unsupported interrupt occured.
 *          other code = error.
 *
 */
int pmpf0100_comms_irq_process(PMPF0100_COMMS *pComms, unsigned int timeout)
{
	int result = EOK;
	struct sigevent timerEvent;
	static int busy=0;
	uint64_t timeoutSetTime, timeoutRemTime;

	if (pComms == NULL)
	{
		return EINVAL;
	}
	if ((pComms->irq_id == -1) || (pComms->gpio_vbase == MAP_DEVICE_FAILED))
	{
		// Bad setup, we should not be doing anything here
		result = EINVAL;
	}
	result = EOK;

	if (timeout > 0)
	{
		if (pComms->verbosity > 5)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
				  "pmpf0100_comms_irq_process waiting with timeout of %d seconds", timeout);
		}

		// Setup the timeout timer
		SIGEV_INTR_INIT (&timerEvent);
		timeoutSetTime = timeout * 1000000000;
		TimerTimeout(CLOCK_REALTIME, _NTO_TIMEOUT_INTR, &timerEvent, &timeoutSetTime, &timeoutRemTime);
	}
	else
	{
		if (pComms->verbosity > 5)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
				  "pmpf0100_comms_irq_process waiting");
		}

	}

	delay(10);

	if (busy) return result;

	busy = 1;
	// Wait for the interrupt event or a timeout
	if (InterruptWait(0, NULL) == -1)
	{
		// A problem occured (not a timeout)
		result = errno;
		if (pComms->verbosity > 2)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
    	          "pmpf0100_comms_irq_process failed InterruptWait (code %d)", result);
		}
	}
	else
	{
		// We get here if the TimerTimeout fires or an interrupt occurs

		if (pComms->verbosity > 5)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
    	          "pmpf0100_comms_irq_process unblocked");
		}

		// Process the event interrupt
		result = pmpf0100_comms_irq_event_process(pComms);

	}
	busy = 0;

	return result;
}



//##############################################################
//    ALL CODE AFTER THIS POINT IS LOCAL TO THIS MODULE
// IT SHOULD NOT BE EXPOSED TO EXTERNAL CODE BY pmpf0100_comms.h
//##############################################################

/*
 * Read either the status, event or IQR mask registers as a single 32-bit value
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_get_status(PMPF0100_COMMS *pComms, uint32_t baseReg, uint32_t *status)
{
	int result = EOK;

	// Make sure the passed info is valid
	if ( (baseReg < PMPF0100_DEVICE_ID     )||
	     (baseReg > PMPF0100_MEMORY_D )  )
	{
		return EINVAL;
	}

	result = pmpf0100_comms_read_register(pComms, baseReg, status);

	return result;
}

/*
 * Set the appropriate status registers using the supplied 32-bit value
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_set_status(PMPF0100_COMMS *pComms, uint32_t baseReg, uint32_t status)
{
	int result = EOK;

	// Make sure the passed info is valid
	if ( (baseReg < PMPF0100_DEVICE_ID     )||
	     (baseReg > PMPF0100_MEMORY_D )  )
	{
		return EINVAL;
	}

    result = pmpf0100_comms_write_register(pComms, baseReg, status);
	return result;
}

/*
 * Write a block of data to the PMPF0100
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_i2c_write(PMPF0100_COMMS *pComms, int incStop,  uint32_t *data, size_t num)
{
	int rbytes;
	int result;

	if ((pComms == NULL) || (data == NULL) || (num == 0) || (num > PMPF0100_SEND_BUFFER_MAX_SIZE))
	{
		return EINVAL;
	}

	pComms->send_msg.hdr.slave.addr = pComms->bus_address;
	pComms->send_msg.hdr.slave.fmt  = I2C_ADDRFMT_7BIT;
	pComms->send_msg.hdr.len = num;
	pComms->send_msg.hdr.stop = ((incStop) ? 1 : 0);
	for (rbytes = 0; rbytes < num; rbytes++)
	{
		pComms->send_msg.data[rbytes] = (uint8_t)data[rbytes];
	}

	rbytes = 0;
	result = devctl(pComms->fd, DCMD_I2C_SEND, &(pComms->send_msg),
                    sizeof(pComms->send_msg) + num, &rbytes);
	if (result != EOK)
	{
		slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
    	          "pmpf0100_comms i2c_write failed (code %d, rbytes %d)", result, rbytes);
	}

	return result;
}


/*
 * Write a block of data to the PMPF0100 and read another block back
 * Returns EOK if no problems, other code if problem
 */
int pmpf0100_comms_i2c_write_read(PMPF0100_COMMS *pComms, int incStop,
		                        uint32_t write_data, uint32_t *read_data)
{
	int result;
	int rbytes;

	if ((pComms == NULL) || (write_data == NULL) || (read_data == NULL))
	{
		return EINVAL;
	}


	pComms->sendrecv_msg.hdr.slave.addr = pComms->bus_address;
	pComms->sendrecv_msg.hdr.slave.fmt  = I2C_ADDRFMT_7BIT;
	pComms->sendrecv_msg.hdr.send_len = 1;
	pComms->sendrecv_msg.hdr.recv_len = 1;
	pComms->sendrecv_msg.hdr.stop = ((incStop) ? 1 : 0);
	rbytes = 1;
    pComms->sendrecv_msg.data[0] = (uint8_t)( write_data & 0xff );

	result = devctl(pComms->fd, DCMD_I2C_SENDRECV, &(pComms->sendrecv_msg),
                    sizeof(pComms->sendrecv_msg), (int *)read_data);

	if (result != EOK)
	{
		slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
    	          "pmpf0100_comms i2c_write_read failed (code %d, rbytes %d)", result, rbytes);
	}

	*read_data = pComms->sendrecv_msg.data[0];

	return result;
}


/*
 * This function scans the event registers and send pulses where needed.
 * Returns EOK if no problems, other code if problem
 *
 * To make sure that there is no interaction between this code and
 *  other code in this module running in different threads, the comms
 *  lock is asserted.
 */
int pmpf0100_comms_irq_event_process(PMPF0100_COMMS *pComms)
{
	int           result;
	uint32_t      currentEvents;
	uint32_t      eventFlags;
	uint32_t      globalEventMask;
	unsigned int  index;
	pmpf0100_event_info_t *pListEntry;

	// Lock access to the callback list
	pthread_mutex_lock(&pComms->event.mutex);

	// Read the PMIC event status registers
	pmpf0100_comms_lock(pComms);
	result = pmpf0100_comms_get_status(pComms, PMPF0100_INT_STATUS0, &currentEvents);
	if (result == EOK)
	{
		// Only clear the ones we have registered an interest in
		globalEventMask = 0;
		for (index = 0; index < pComms->event.listInUse; index++)
		{
			globalEventMask |= pComms->event.list[index].pulse.eventMask;
		}
		result = pmpf0100_comms_set_status(pComms, PMPF0100_INT_STATUS0, globalEventMask);

		if (pComms->verbosity > 5)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
					"pmpf0100_comms_irq_event_process processing 0x%08X (cleared 0x%08X)",
					currentEvents, globalEventMask);
		}
	}
	pmpf0100_comms_unlock(pComms);

	// Clear the mask AFTER clearing the event flags or we will get repeated interrupts
	if (InterruptUnmask(pComms->irq_number, pComms->irq_id) == -1)
	{
		slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
			  "pmpf0100_comms_irq_event_process failed IRQ unmask (code %d)", errno);
	}

	// For each registered event send a suitable pulse
	if (result == EOK)
	{
		for (index=0; index < pComms->event.listInUse; index++)
		{
			pListEntry = &pComms->event.list[index];
			// See if we have something to do
			eventFlags = currentEvents & pListEntry->pulse.eventMask;
			if (eventFlags != 0)
			{
				// We have an event to process so send the appropriate pulse
				if (pComms->verbosity > 4)
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
							"pmpf0100_comms_irq_event_process  index=%d, mask=0x%08X, pid=%d, chid=%d, coid=%d, pri=%d, code=%d, GUID=%d",
							index, pListEntry->pulse.eventMask,
							pListEntry->pulse.ownerId,
							pListEntry->pulse.channelId,
							pListEntry->connectionId,
							pListEntry->pulse.priority,
							pListEntry->pulse.code,
							pListEntry->pulse.guid);
				}

				if (MsgSendPulse(pListEntry->connectionId, pListEntry->pulse.priority,
				                 pListEntry->pulse.code, eventFlags) == -1)
				{
					result = errno;
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO,
							"pmpf0100_comms_irq_event_process MagSendPulse failed for event mask 0x%08X (code %d)",
							eventFlags, result);
				}

			}

		}

	}

	// Unlock access to the callback list
	pthread_mutex_unlock(&pComms->event.mutex);

	return result;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/support/pmic_pmpf0100/pmpf0100_comms.c $ $Rev: 763655 $")
#endif
