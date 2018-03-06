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


#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <devctl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>

#include "pmpf0100_comms.h"



int    decode_command_line(int argc, char *argv[]);
int    io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb);
int    pmic_irq_init(void);
int    pmic_irq_uninit(void);
void * pmic_irq_thread(void *arg);



typedef struct
{
	char                      device_name[128];
	unsigned int              bus_number;
	unsigned int              bus_address;
	int	                      verbosity;
	PMPF0100_COMMS             *pPmicComms;
	resmgr_connect_funcs_t    connect_funcs;
	resmgr_io_funcs_t         io_funcs;
	iofunc_attr_t             attr;
	int                       irq_number;
	pthread_t                 irq_thread;
	int                       irq_thread_stop;
} pmpf0100_pmic_dev_t;

static pmpf0100_pmic_dev_t info;

void dump(void)
{
    uint32_t regValue[8];
	int      i;
	int      result = EOK;

	for (i=0; i<64; i+=8)
	{
		regValue[0]=0;
		regValue[1]=0;
		regValue[2]=0;
		regValue[3]=0;
		regValue[4]=0;
		regValue[5]=0;
		regValue[6]=0;
		regValue[7]=0;

		result = pmpf0100_comms_read_register(info.pPmicComms, i, &regValue[0]);
		if (result != EOK)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					"DCMD_PMPF0100_REGISTER_READ request failed (code %d)", result);
		}
		result = pmpf0100_comms_read_register(info.pPmicComms, i+1, &regValue[1]);
		if (result != EOK)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					"DCMD_PMPF0100_REGISTER_READ request failed (code %d)", result);
		}
		result = pmpf0100_comms_read_register(info.pPmicComms, i+2, &regValue[2]);
		if (result != EOK)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					"DCMD_PMPF0100_REGISTER_READ request failed (code %d)", result);
		}
		result = pmpf0100_comms_read_register(info.pPmicComms, i+3, &regValue[3]);
		if (result != EOK)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					"DCMD_PMPF0100_REGISTER_READ request failed (code %d)", result);
		}
		result = pmpf0100_comms_read_register(info.pPmicComms, i+4, &regValue[4]);
		if (result != EOK)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					"DCMD_PMPF0100_REGISTER_READ request failed (code %d)", result);
		}
		result = pmpf0100_comms_read_register(info.pPmicComms, i+5, &regValue[5]);
		if (result != EOK)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					"DCMD_PMPF0100_REGISTER_READ request failed (code %d)", result);
		}
		result = pmpf0100_comms_read_register(info.pPmicComms, i+6, &regValue[6]);
		if (result != EOK)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					"DCMD_PMPF0100_REGISTER_READ request failed (code %d)", result);
		}
		result = pmpf0100_comms_read_register(info.pPmicComms, i+7, &regValue[7]);
		if (result != EOK)
		{
			slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					"DCMD_PMPF0100_REGISTER_READ request failed (code %d)", result);
		}
		printf("REGS%02d: %06x %06x %06x %06x " , i, regValue[0], regValue[1], regValue[2], regValue[3]);
		printf(          "%06x %06x %06x %06x\n",    regValue[4], regValue[5], regValue[6], regValue[7]);
	}
}

/*
 * Main entry point for driver load utility
 */
int main(int argc, char *argv[])
{
    resmgr_attr_t        resmgr_attr;
    dispatch_t           *dpp;
    dispatch_context_t   *ctp;
    int                  id;
    int                  result = EOK;

    // Decode the command line
    result = decode_command_line(argc, argv);
    if (result != EOK)
    {
    	return result;
    }

    // Open comms with the PMIC
    result = pmpf0100_comms_open(info.bus_number, info.bus_address, &info.pPmicComms);
    if ( result != EOK)
    {
    	fprintf(stderr, "  ERROR: %s cannot open comms to PMIC (code %d)\n",
    			argv[0], result);
    	return result;
    }

    // Set comms verbosity and speed
    result = pmpf0100_comms_set_verbosity(info.pPmicComms, info.verbosity);

	// Mask out all PMIC interrupts and clear all events
	pmpf0100_comms_lock(info.pPmicComms);

	pmpf0100_comms_write_register(info.pPmicComms, PMPF0100_INT_MASK0, PMPF0100_INT_MASK0_DEF);
	pmpf0100_comms_write_register(info.pPmicComms, PMPF0100_INT_STATUS0, PMPF0100_INT_MASK0_DEF);
    
	pmpf0100_comms_write_register(info.pPmicComms, PMPF0100_INT_MASK1, PMPF0100_INT_MASK1_DEF);
	pmpf0100_comms_write_register(info.pPmicComms, PMPF0100_INT_STATUS1, PMPF0100_INT_MASK1_DEF);

	pmpf0100_comms_write_register(info.pPmicComms, PMPF0100_INT_MASK3, PMPF0100_INT_MASK3_DEF);
	pmpf0100_comms_write_register(info.pPmicComms, PMPF0100_INT_STATUS3, PMPF0100_INT_MASK3_DEF);

	pmpf0100_comms_write_register(info.pPmicComms, PMPF0100_INT_MASK4, PMPF0100_INT_MASK4_DEF);
	pmpf0100_comms_write_register(info.pPmicComms, PMPF0100_INT_STATUS4, PMPF0100_INT_MASK4_DEF);

	pmpf0100_comms_unlock(info.pPmicComms);

	// Initialize the dispatch interface
    if (result == EOK)
    {

		dpp = dispatch_create();
		if (dpp == NULL)
		{
			fprintf(stderr, "  ERROR: %s unable to allocate dispatch handle.\n",
					argv[0]);
			result = -1;
		}
    }

    // Initialize resource manager
    if (result == EOK)
    {
		memset(&resmgr_attr, 0, sizeof resmgr_attr);
		resmgr_attr.nparts_max = 1;
		resmgr_attr.msg_max_size = 512;  // The biggest block of data to be transferred

		// Initialize the function structures for handling messages
		iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &info.connect_funcs,
						 _RESMGR_IO_NFUNCS,      &info.io_funcs);

		// Handle _IO_DEVCTL, sent by devctl()
		info.io_funcs.devctl = io_devctl;

		// Initialize the attribute structure used by the device
		iofunc_attr_init(&info.attr, S_IFNAM | 0666, 0, 0);

		// Attach our device name
		id = resmgr_attach(dpp,                    /* dispatch handle        */
						   &resmgr_attr,           /* resource manager attrs */
						   info.device_name,       /* device name            */
						   _FTYPE_ANY,             /* open type              */
						   0,                      /* flags                  */
						   &info.connect_funcs,    /* connect routines       */
						   &info.io_funcs,         /* I/O routines           */
						   &info.attr);            /* handle                 */
		if (id == -1)
		{
			fprintf(stderr, "  ERROR: %s unable to attach device name %s\n",
					argv[0], info.device_name);
			result = -1;
		}
    }

    // Allocate a context structure
    if (result == EOK)
   	{
    	ctp = dispatch_context_alloc(dpp);
    	if (ctp == NULL)
    	{
    		fprintf(stderr, "  ERROR: %s unable to allocate context (code %d)\n",
    				argv[0], errno);
    		result = -1;
    	}
   	}

    // Setup the interrupt handler and run the message loop
    if (result == EOK)
   	{
   		result = pmic_irq_init();
   		if (result != EOK)
   		{
   	        fprintf(stderr, "  ERROR: %s unable to initialise IRQ (code %d)\n",
   	                argv[0], result);
   			result = -1;
   		}
   		else
   		{
			if (info.verbosity > 4)
			{
				fprintf(stderr, "  %s entering main message loop\n",
						argv[0]);
			}

			// Enter the resource manager message loop
			while (result == EOK)
			{
				ctp = dispatch_block(ctp);
				if (ctp == NULL)
				{
					fprintf(stderr, "  ERROR: %s block error\n",
							argv[0]);
					result = -1;
				}
				else
				{
					dispatch_handler(ctp);
				}
			}

			if (info.verbosity > 4)
			{
				fprintf(stderr, "  %s exited main message loop\n",
						argv[0]);
			}

			// Disconnect the interrupt handler
			pmic_irq_uninit();
   		}
   	}

    // Close the comms interface
    if (info.pPmicComms != NULL)
    {
    	pmpf0100_comms_close(&info.pPmicComms);
    }

    return result;
}

/*
 * Decode the command line options
 */
int decode_command_line(int argc, char *argv[])
{
    int     			c;
    int     			done = 0;
    int                 result = EOK;

    // Initialise default settings
    info.pPmicComms = NULL;
	info.irq_thread = -1;
	info.irq_number = -1;
    info.bus_number = PMPF0100_DEFAULT_2WIRE_BUS;
    info.bus_address = PMPF0100_DEFAULT_2WIRE_ADDRESS;
    strcpy(info.device_name, PMPF0100_DEVICE_NAME);
    info.verbosity = 0;

    while (!done)
    {
        c = getopt(argc, argv, "b:a:i:v:s:");
        switch (c)
        {
        	case -1:
        		done = 1;
        		break;
        	case 'b':
        		info.bus_number = strtoul(optarg, &optarg, NULL);
        		break;
        	case 'a':
        		info.bus_address = strtoul(optarg, &optarg, NULL);
        		break;
        	case 'i':
        		info.irq_number = strtoul(optarg, &optarg, NULL);
        		break;
        	case 'v':
        		info.verbosity =  strtoul(optarg, &optarg, NULL);
        		break;
        	default:
            	fprintf(stderr, "  ERROR: %s unknown option '%c'\n", argv[0], c);
        		break;
        }

    }

    if (info.verbosity > 0)
    {
    	fprintf(stderr, "PMPF0100 PMIC Driver Settings:\n");
    	fprintf(stderr, "  Device name .......: %s\n", info.device_name);
    	fprintf(stderr, "  I2C bus number ....: %d\n", info.bus_number);
    	fprintf(stderr, "  I2C bus address ...: %d\n", info.bus_address);
    	if (info.irq_number == -1)
    	{
        	fprintf(stderr, "  IRQ number ........: GPIO4[16] system default\n");
    	}
    	else
    	{
        	fprintf(stderr, "  IRQ number ........: %d\n", info.irq_number);
    	}
    	fprintf(stderr, "  Verbosity Level ...: %d\n", info.verbosity);
    }

	return result;
}


/*
 * The devctl handler for the /dev/pmic device
 */
int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb) 
{
    int result = EOK;
    void *pMsgData;

    /*
     Note this assumes that you can fit the entire data portion of
     the devctl into one message.  In reality you should probably
     perform a MsgReadv() once you know the type of message you
     have received to suck all of the data in rather than assuming
     it all fits in the message.
    */
    if (info.verbosity > 6)
    {
    	slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO, "PMIC got devctl message");
    	slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO, "  PMIC type ..: %d", msg->i.type);
    	slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO, "  PMIC clen ..: %d", msg->i.combine_len);
    	slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO, "  PMIC dcmd ..: %d", msg->i.dcmd);
    	slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO, "  PMIC nbytes : %d", msg->i.nbytes);
    }

    pMsgData = NULL;
    if (msg->i.nbytes > 0)
    {
    	pMsgData = _DEVCTL_DATA(msg->i);
    }

    switch (msg->i.dcmd) 
	{
    	case DCMD_PMPF0100_LOCK:
    	    result = pmpf0100_comms_lock(info.pPmicComms);
    		break;

    	case DCMD_PMPF0100_UNLOCK:
    	    result = pmpf0100_comms_unlock(info.pPmicComms);
    		break;

    	case DCMD_PMPF0100_DRIVER_INFO:
    		{
    			pmic_version_info_t *rsp;

				if ((pMsgData == NULL) || (msg->i.nbytes != sizeof(pmic_version_info_t)))
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_DRIVER_INFO request EINVAL");
					return EINVAL;
				}
				rsp = (pmic_version_info_t *)_DEVCTL_DATA(msg->o);
				memset(&msg->o, 0, sizeof(msg->o));
				msg->o.ret_val = sizeof(pmic_version_info_t);
				msg->o.nbytes = sizeof(pmic_version_info_t);
				rsp->version_major = PMPF0100_DRIVER_VERSION_MAJOR;
				rsp->version_minor = PMPF0100_DRIVER_VERSION_MINOR;
    		}
    	    break;

    	case DCMD_PMPF0100_REGISTER_READ:
    	    {
    	    	pmic_send_recv_t  req;
    	    	pmic_send_recv_t *rsp;

				if ((pMsgData == NULL) || (msg->i.nbytes != sizeof(pmic_send_recv_t)))
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_REGISTER_READ request EINVAL");
					return EINVAL;
				}
				req = *(pmic_send_recv_t *)pMsgData;
				result = pmpf0100_comms_read_register(info.pPmicComms, req.reg_number, &req.reg_data);
				if (result != EOK)
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_REGISTER_READ request failed (code %d)", result);
					return result;
				}
				else
				{
					rsp = (pmic_send_recv_t *)_DEVCTL_DATA(msg->o);
					memset(&msg->o, 0, sizeof(msg->o));
					msg->o.ret_val = sizeof(pmic_send_recv_t);
					msg->o.nbytes = sizeof(pmic_send_recv_t);
					rsp->reg_number = req.reg_number;
					rsp->reg_data = req.reg_data;
				}
			}
    		break;

    	case DCMD_PMPF0100_REGISTER_WRITE:
    		{
    			pmic_send_recv_t req;

    			if ((pMsgData == NULL) || (msg->i.nbytes != sizeof(pmic_send_recv_t)))
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_REGISTER_WRITE request EINVAL");
					return EINVAL;
				}
    			req = *(pmic_send_recv_t *)pMsgData;
				result = pmpf0100_comms_write_register(info.pPmicComms, req.reg_number, req.reg_data);
				if (result != EOK)
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_REGISTER_WRITE request failed (code %d)", result);
					return result;
				}
    		}
    		break;

    	case DCMD_PMPF0100_STATUS_READ:
    		{
    			uint32_t  status;

    			if ((pMsgData == NULL) || (msg->i.nbytes != sizeof(uint32_t)))
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_STATUS_READ request EINVAL");
					return EINVAL;
				}
				result = pmpf0100_comms_read_status(info.pPmicComms, &status);
				if (result != EOK)
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_REGISTER_WRITE request code %d", result);
				}
				else
				{
					memset(&msg->o, 0, sizeof(msg->o));
					msg->o.nbytes = sizeof(uint32_t);
					msg->o.ret_val = status;
				}
    		}
    		break;

    	case DCMD_PMPF0100_EVENT_PULSE_REGISTER:
    		{
    			pmic_pulse_info_t  req;
    			pmic_pulse_info_t *rsp;

				if ((pMsgData == NULL) || (msg->i.nbytes != sizeof(pmic_pulse_info_t)))
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_EVENT_PULSE_REGISTER request EINVAL");
					return EINVAL;
				}
				req = *(pmic_pulse_info_t *)pMsgData;
				result = pmpf0100_comms_event_register(info.pPmicComms, &req);
				if (result != EOK)
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_EVENT_PULSE_REGISTER register failed (code %d)", result);
				}
				else
				{
					rsp = (pmic_pulse_info_t *)_DEVCTL_DATA(msg->o);
					memset(&msg->o, 0, sizeof(msg->o));
					msg->o.ret_val = sizeof(uint32_t);
					msg->o.nbytes = sizeof(uint32_t);
					*rsp = req;
				}
    		}
    		break;

    	case DCMD_PMPF0100_EVENT_PULSE_CANCEL:
			if ((pMsgData == NULL) || (msg->i.nbytes != sizeof(uint32_t)))
			{
				slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
				      "DCMD_PMPF0100_EVENT_PULSE_CANCEL request EINVAL");
				return EINVAL;
			}
			result = pmpf0100_comms_event_cancel(info.pPmicComms, *(uint32_t *)pMsgData);
			if (result != EOK)
			{
				slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
				      "DCMD_PMPF0100_EVENT_PULSE_CANCEL failed (code %d)", result);
			}
    		break;

    	case DCMD_PMPF0100_ATOD_RESOLUTION:
			{
    			uint32_t *rsp;

    			if ((pMsgData == NULL) || (msg->i.nbytes != sizeof(uint32_t)))
				{
					slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR,
					      "DCMD_PMPF0100_ATOD_RESOLUTION request EINVAL");
					return EINVAL;
				}
				rsp = (uint32_t *)_DEVCTL_DATA(msg->o);
				memset(&msg->o, 0, sizeof(msg->o));
				msg->o.ret_val = sizeof(uint32_t);
				msg->o.nbytes = sizeof(uint32_t);
				*rsp = 0x3FF;  // 10-bit resolution
			}
			break;

    	default:
            // See if any default actions
        	return iofunc_devctl_default(ctp, msg, ocb);
            break;
    }

    if (result == EOK)
    {
		result = _RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + msg->o.nbytes);
    }
    
	return result;
}

/*
 * Initialise the interrupt handling.
 * All IRQ setup and handling is done in a separate thread.
 * If it is not already running, this code starts it.
 */
int pmic_irq_init(void)
{
	int result = EOK;

	if (info.irq_number == -1)
	{
		result = pmpf0100_comms_get_default_irq(info.pPmicComms, &info.irq_number);
	}
	if (result == EOK)
	{
		if (info.irq_thread == -1)
		{
			info.irq_thread_stop = 0;
			result = pthread_create(&info.irq_thread, NULL, pmic_irq_thread, &info);
			if (result != EOK)
			{
				slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR, "  PMIC IRQ thread create failed (code %d)", result);
				info.irq_thread = -1;
			}
		}
	}

	return result;
}

/*
 * Un-Initialise the interrupt handling
 * Shuts down the IRQ processing thread.
 */
int pmic_irq_uninit(void)
{
	int result = EOK;

	if (info.irq_thread != -1)
	{
		info.irq_thread_stop = 1;

		// Forcibly stop the thread TODO
		pthread_cancel(info.irq_thread);

		info.irq_thread = -1;
	}

	return result;
}

/*
 * Thread used to setup and process PMIC interrupts
 */
void * pmic_irq_thread(void *arg)
{
	pmpf0100_pmic_dev_t *devInfo = (pmpf0100_pmic_dev_t *)arg;
	int result;

	result = pmpf0100_comms_irq_init(devInfo->pPmicComms, &devInfo->irq_number, PMPF0100_IRQ_FALLING_EDGE);
	if (result != EOK)
	{
		slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_ERROR, "PMIC comms irq init failed (code %d)", result);
	}
	else
	{
		// Service the PMIC interrupts when they occur
		while (!devInfo->irq_thread_stop)
		{
			pmpf0100_comms_irq_process(devInfo->pPmicComms, 1);
		}

		pmpf0100_comms_irq_uninit(devInfo->pPmicComms);
	}

	slogf(_SLOG_SETCODE(_SLOGC_INPUT, 1), _SLOG_INFO, "PMIC irq thread exit (code %d)", result);

	return (void *)result;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/support/pmic_pmpf0100/main.c $ $Rev: 750274 $")
#endif
