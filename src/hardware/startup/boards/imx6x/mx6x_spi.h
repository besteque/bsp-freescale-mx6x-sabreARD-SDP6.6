/*
 * $QNXLicenseC:
 * Copyright 2016, QNX Software Systems.
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

#ifndef __MX6X_SPI_H_INCLUDED
#define __MX6X_SPI_H_INCLUDED

#include <stdint.h>
#include <stddef.h>

#define MX6X_SPI_INTERNAL_SIZE 16

typedef struct {
    uint8_t sclk_idle_high; /* sclk_ctl */
    uint8_t data_idle_high; /* data_ctl */
    uint8_t ss_active_high; /* ss_pol   */
    uint8_t ss_multi_burst; /* ss_ctl   */
    uint8_t sclk_pol_low;   /* sclk_pol */
    uint8_t sclk_pha;       /* sclk_pha */
} spi_channel_config_t;

typedef struct {
    uint64_t base;
    size_t channel;
    spi_channel_config_t config;
    uint32_t clock_rate;

    /* only used internally by mx6x_spi code */
    uint8_t internal[MX6X_SPI_INTERNAL_SIZE];
} spi_dev_t;

int spi_init(spi_dev_t *dev);
int spi_xfer(spi_dev_t *dev, size_t len, uint8_t *data);

#endif /* __MX6X_SPI_H_INCLUDED */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/mx6x_spi.h $ $Rev: 804201 $")
#endif
