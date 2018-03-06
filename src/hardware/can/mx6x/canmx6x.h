/*
 * $QNXLicenseC:
 * Copyright 2015, QNX Software Systems.
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

#ifndef _CANMX6X_H_INCLUDED
#define _CANMX6X_H_INCLUDED

#include <hw/mx6x-can.h>

/*
 * Note: ARM platforms running >= 6.6.0 need to execute at a lower privilege level
 *       CAN registers are otherwise inaccessible (bus error).
 */
#if defined(__ARM__) && defined(_NTO_TCTL_IO_PRIV)
#define PRIVITY_FLAGS    _NTO_TCTL_IO_PRIV
#else
#define PRIVITY_FLAGS    _NTO_TCTL_IO
#endif

/* Driver implemented CAN library function prototypes */
void can_drvr_transmit(CANDEV *cdev);
int can_drvr_devctl(CANDEV *cdev, int dcmd, DCMD_DATA *data);

/* Function prototypes */
void can_options(CANDEV_RINGO_INIT *devinit, int argc, char *argv[]);
void can_init_mailbox(CANDEV_RINGO_INFO *devinfo, CANDEV_RINGO_INIT *devinit);
void can_init_hw(CANDEV_RINGO_INFO *devinfo, CANDEV_RINGO_INIT *devinit);
void can_init_intr(CANDEV_RINGO_INFO *devinfo, CANDEV_RINGO_INIT *devinit, uint32_t mdriver_intr);
void can_print_reg(CANDEV_RINGO_INFO *devinfo);
void can_print_mailbox(CANDEV_RINGO_INFO *devinfo);
void set_port32(unsigned port, uint32_t mask, uint32_t data);

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/can/mx6x/canmx6x.h $ $Rev: 781112 $")
#endif
