/*
 * $QNXLicenseC:
 * Copyright 2012, QNX Software Systems.
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

#define ESAI_MAIN_CLK_SRC   MAIN_CLK_SRC_OSC

#define SAMPLE_RATE_MIN       48000
#define SAMPLE_RATE_MAX       48000
#define AIF_TX_VOICES         "8"
#define AIF_RX_VOICES         "2:2"

#define SDO_PIN_MAP           "0:1:2:3"
#define SDI_PIN_MAP           "0:1"
#define SDI_RECONSTITUTE_MAP  "2:0:1:3"   /* The Mic inputs are wired backwards so we must swap 0 and 2 in the map */

#define ESAI_BASE_ADDR        0x02024000

#define ESAI_TX_DMA_EVENT     24
#define ESAI_RX_DMA_EVENT     23

#define ESAI_TX_DMA_CTYPE     3	/* SDMA_CHTYPE_MCU_2_SHP from private header sdma.h */
#define ESAI_RX_DMA_CTYPE     4	/* SDMA_CHTYPE_SHP_2_MCU from private header sdma.h */

#define I2C_BUS_NUMBER        2
#define I2C_SLAVE_ADDR        0x48

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/deva/ctrl/mxesai/nto/arm/dll.le.v7.mx6sabreARD/variant.h $ $Rev: 808011 $")
#endif
