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

#ifndef INCLUDE_PMPF0100_COMMS_H_
#define INCLUDE_PMPF0100_COMMS_H_

#include <hw/pmpf0100_pmic.h>

// The QNX GPIO4 MSB virtual interrupt base
#define QNX_GPIO4_16_IRQ              256


typedef enum
{
	PMPF0100_IRQ_LEVEL_LOW = 0,
	PMPF0100_IRQ_LEVEL_HIGH,
	PMPF0100_IRQ_FALLING_EDGE,
	PMPF0100_IRQ_RISING_EDGE
} PMPF0100_IRQ_MODE;

typedef struct pmpf0100_comms_t PMPF0100_COMMS;

int pmpf0100_comms_open(unsigned i2c_bus, unsigned int i2c_address, PMPF0100_COMMS **ppComms);
int pmpf0100_comms_close(PMPF0100_COMMS **ppComms);

int pmpf0100_comms_lock(PMPF0100_COMMS *pComms);
int pmpf0100_comms_unlock(PMPF0100_COMMS *pComms);

int pmpf0100_comms_get_default_irq(PMPF0100_COMMS *pComms, int *irqNumber);

int pmpf0100_comms_set_verbosity(PMPF0100_COMMS *pComms, int verbosity);
int pmpf0100_comms_set_bus_speed(PMPF0100_COMMS *pComms, unsigned int bus_speed);

int pmpf0100_comms_read_register(PMPF0100_COMMS *pComms, uint32_t regNum, uint32_t *regValue);
int pmpf0100_comms_write_register(PMPF0100_COMMS *pComms, uint32_t regNum, uint32_t regValue);
int pmpf0100_comms_read_status(PMPF0100_COMMS *pComms, uint32_t *regValue);

int pmpf0100_comms_irq_init(PMPF0100_COMMS *pComms, int *irqNumber, PMPF0100_IRQ_MODE irqMode);
int pmpf0100_comms_irq_uninit(PMPF0100_COMMS *pComms);
int pmpf0100_comms_irq_process(PMPF0100_COMMS *pComms, unsigned int timeout);

int pmpf0100_comms_event_register(PMPF0100_COMMS *pComms, pmic_pulse_info_t *pCallback);
int pmpf0100_comms_event_cancel(PMPF0100_COMMS *pComms, uint32_t callBackId);


#endif //INCLUDE_PMPF0100_COMMS_H_

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/support/pmic_pmpf0100/pmpf0100_comms.h $ $Rev: 680881 $")
#endif
