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

#ifndef _MINI_CAN_H_INCLUDED
#define _MINI_CAN_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#define CAN_RX_ALL (UINT32_MAX)

typedef enum {
	MINICAN_BAUD_1M,
	MINICAN_BAUD_500K,
	MINICAN_BAUD_250K,
	MINICAN_BAUD_125K,
	MINICAN_BAUD_50K,
} minican_baud_t;

typedef struct {
	uint32_t id;
	bool     is_extended;
} minican_id_t;

typedef struct {
	minican_id_t arb;
	size_t       len;
	uint8_t      data[8];
} minican_msg_t;

typedef struct {
	size_t         count;
	minican_msg_t *buf;
} minican_tx_msg_list_t;

typedef struct {
	size_t        count;
	minican_id_t *buf;
} minican_rx_msg_list_t;

typedef struct {
	paddr_t can_base;
	paddr_t canmem_base;
	size_t can_irq;
	minican_baud_t rate;
	minican_rx_msg_list_t msgs_to_rx;
	minican_tx_msg_list_t init_tx_msgs;
} minican_config_t;

/* minidriver is expecting this configuration and all lists within it to be
 * either static or global variables. It stores a pointer to them.
 */
int mini_can_preconfigure(minican_config_t const * const _config, paddr_t mdriver_data);
int mini_can(int state, void *data);

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/mini-can/mini_can.h $ $Rev: 804160 $")
#endif
