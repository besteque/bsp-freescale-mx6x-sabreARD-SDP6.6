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

#include "arm/mx6x.h"
#include "mx6x_spi.h"
#include "mx6x_startup.h"
#include <hw/inout.h>

#define ECSPI_CHANNEL(n)                       (1 << (n))

#define MX6X_ECSPI_CONREG_EN                   (1)
#define MX6X_ECSPI_CONREG_XCH                  (4)
#define MX6X_ECSPI_CONREG_SMC                  (8)
#define MX6X_ECSPI_CONREG_CHANNEL_MODE_SHIFT   (4)
#define MX6X_ECSPI_CONREG_CHANNEL_MODE_MASK    (0xF << MX6X_ECSPI_CONREG_CHANNEL_MODE_SHIFT)
#define MX6X_ECSPI_CONREG_POST_DIVIDER_SHIFT   (8)
#define MX6X_ECSPI_CONREG_POST_DIVIDER_MASK    (0xF << MX6X_ECSPI_CONREG_POST_DIVIDER_SHIFT)
#define MX6X_ECSPI_CONREG_PRE_DIVIDER_SHIFT    (12)
#define MX6X_ECSPI_CONREG_PRE_DIVIDER_MASK     (0xF << MX6X_ECSPI_CONREG_PRE_DIVIDER_SHIFT)
#define MX6X_ECSPI_CONREG_DRCTL_SHIFT          (16)
#define MX6X_ECSPI_CONREG_DRCTL_MASK           (0x3 << MX6X_ECSPI_CONREG_DRCTL_SHIFT)
#define MX6X_ECSPI_CONREG_CHANNEL_SELECT_SHIFT (18)
#define MX6X_ECSPI_CONREG_CHANNEL_SELECT_MASK  (0x3 << MX6X_ECSPI_CONREG_CHANNEL_SELECT_SHIFT)
#define MX6X_ECSPI_CONREG_BURST_LENGTH_SHIFT   (20)
#define MX6X_ECSPI_CONREG_BURST_LENGTH_MASK    (0xFFF << MX6X_ECSPI_CONREG_BURST_LENGTH_SHIFT)

#define MX6X_ECSPI_CONFIGREG_SCLK_PHA_SHIFT    (0)
#define MX6X_ECSPI_CONFIGREG_SCLK_POL_SHIFT    (4)
#define MX6X_ECSPI_CONFIGREG_SS_CTL_SHIFT      (8)
#define MX6X_ECSPI_CONFIGREG_SS_POL_SHIFT      (12)
#define MX6X_ECSPI_CONFIGREG_DATA_CTL_SHIFT    (16)
#define MX6X_ECSPI_CONFIGREG_SCLK_CTL_SHIFT    (20)

#define MX6X_ECSPI_DMAREG_TX_THRESHOLD_SHIFT   (0)
#define MX6X_ECSPI_DMAREG_TX_THRESHOLD_MASK    (0x3F << MX6X_ECSPI_DMAREG_TX_THRESHOLD_SHIFT)
#define MX6X_ECSPI_DMAREG_RX_THRESHOLD_SHIFT   (16)
#define MX6X_ECSPI_DMAREG_RX_THRESHOLD_MASK    (0x3F << MX6X_ECSPI_DMAREG_RX_THRESHOLD_SHIFT)

#define MX6X_ECSPI_STATREG_TE                  (1 << 0)
#define MX6X_ECSPI_STATREG_TDR                 (1 << 1)
#define MX6X_ECSPI_STATREG_TF                  (1 << 2)
#define MX6X_ECSPI_STATREG_RR                  (1 << 3)
#define MX6X_ECSPI_STATREG_RDR                 (1 << 4)
#define MX6X_ECSPI_STATREG_RF                  (1 << 5)
#define MX6X_ECSPI_STATREG_RO                  (1 << 6)
#define MX6X_ECSPI_STATREG_TC                  (1 << 7)

#define MX6X_ECSPI_TESTREG_TXCNT_SHIFT         (0)
#define MX6X_ECSPI_TESTREG_TXCNT_MASK          (0x7F << MX6X_ECSPI_TESTREG_TXCNT_SHIFT)
#define MX6X_ECSPI_TESTREG_RXCNT_SHIFT         (8)
#define MX6X_ECSPI_TESTREG_RXCNT_MASK          (0x7F << MX6X_ECSPI_TESTREG_RXCNT_SHIFT)
#define MX6X_ECSPI_TESTREG_LBC                 (1 << 31)

#define RX_WATERMARK (32)
#define TX_WATERMARK (32)

typedef struct {
    uint32_t conreg;
    uint32_t conreg_mask;
    uint32_t configreg;
    uint32_t configreg_mask;
} calculated_reg_vals_t;

/* Just want to make sure that MX6X_SPI_INTERNAL_SIZE defines enough bytes to
 * hold calculated_reg_vals_t. If MX6X_SPI_INTERNAL_SIZE is too small the next
 * line will break the compile.
 */
extern char __CHECK__[1/!(sizeof(calculated_reg_vals_t) > MX6X_SPI_INTERNAL_SIZE)];

static size_t read_word(spi_dev_t *dev, uint8_t *data, size_t received_length, size_t len);

int spi_init(spi_dev_t *dev) {
    calculated_reg_vals_t *reg_vals = (calculated_reg_vals_t*)&dev->internal[0];
    uint32_t post_div, pre_div, drate, post_drate, module_clock = mx6x_get_ecspi_clk();

    for (post_div = 0; post_div < 16 ; post_div++) {
        post_drate = module_clock >> post_div;
        for (pre_div = 0; pre_div < 16 ; pre_div++) {
            drate = post_drate / (pre_div + 1);
            if (drate <= dev->clock_rate) {
                break;
            }
        }
        if (drate <= dev->clock_rate) {
            break;
        }
    }

    reg_vals->conreg = 0;
    reg_vals->conreg |= (dev->channel << MX6X_ECSPI_CONREG_CHANNEL_SELECT_SHIFT) & MX6X_ECSPI_CONREG_CHANNEL_SELECT_MASK;
    reg_vals->conreg |= (pre_div << MX6X_ECSPI_CONREG_PRE_DIVIDER_SHIFT) & MX6X_ECSPI_CONREG_PRE_DIVIDER_MASK;
    reg_vals->conreg |= (post_div << MX6X_ECSPI_CONREG_POST_DIVIDER_SHIFT) & MX6X_ECSPI_CONREG_POST_DIVIDER_MASK;
    reg_vals->conreg |= ECSPI_CHANNEL(dev->channel) << MX6X_ECSPI_CONREG_CHANNEL_MODE_SHIFT;
    reg_vals->conreg |= MX6X_ECSPI_CONREG_SMC;
    reg_vals->conreg |= MX6X_ECSPI_CONREG_EN;

    reg_vals->conreg_mask = 0;
    reg_vals->conreg_mask |= MX6X_ECSPI_CONREG_CHANNEL_SELECT_MASK;
    reg_vals->conreg_mask |= MX6X_ECSPI_CONREG_DRCTL_MASK;
    reg_vals->conreg_mask |= MX6X_ECSPI_CONREG_PRE_DIVIDER_MASK;
    reg_vals->conreg_mask |= MX6X_ECSPI_CONREG_POST_DIVIDER_MASK;
    reg_vals->conreg_mask |= ECSPI_CHANNEL(dev->channel) << MX6X_ECSPI_CONREG_CHANNEL_MODE_SHIFT;
    reg_vals->conreg_mask |= MX6X_ECSPI_CONREG_SMC;
    reg_vals->conreg_mask |= MX6X_ECSPI_CONREG_XCH;
    reg_vals->conreg_mask |= MX6X_ECSPI_CONREG_EN;

    reg_vals->configreg = 0;
    reg_vals->configreg |= (!!dev->config.sclk_pha) << (dev->channel + MX6X_ECSPI_CONFIGREG_SCLK_PHA_SHIFT);
    reg_vals->configreg |= (!!dev->config.sclk_pol_low) << (dev->channel + MX6X_ECSPI_CONFIGREG_SCLK_POL_SHIFT);
    reg_vals->configreg |= (!!dev->config.ss_multi_burst) << (dev->channel + MX6X_ECSPI_CONFIGREG_SS_CTL_SHIFT);
    reg_vals->configreg |= (!!dev->config.ss_active_high) << (dev->channel + MX6X_ECSPI_CONFIGREG_SS_POL_SHIFT);
    reg_vals->configreg |= (!!dev->config.data_idle_high) << (dev->channel + MX6X_ECSPI_CONFIGREG_DATA_CTL_SHIFT);
    reg_vals->configreg |= (!!dev->config.sclk_idle_high) << (dev->channel + MX6X_ECSPI_CONFIGREG_SCLK_CTL_SHIFT);

    reg_vals->configreg_mask = 0;
    reg_vals->configreg_mask |= ECSPI_CHANNEL(dev->channel) << MX6X_ECSPI_CONFIGREG_SCLK_PHA_SHIFT;
    reg_vals->configreg_mask |= ECSPI_CHANNEL(dev->channel) << MX6X_ECSPI_CONFIGREG_SCLK_POL_SHIFT;
    reg_vals->configreg_mask |= ECSPI_CHANNEL(dev->channel) << MX6X_ECSPI_CONFIGREG_SS_CTL_SHIFT;
    reg_vals->configreg_mask |= ECSPI_CHANNEL(dev->channel) << MX6X_ECSPI_CONFIGREG_SS_POL_SHIFT;
    reg_vals->configreg_mask |= ECSPI_CHANNEL(dev->channel) << MX6X_ECSPI_CONFIGREG_DATA_CTL_SHIFT;
    reg_vals->configreg_mask |= ECSPI_CHANNEL(dev->channel) << MX6X_ECSPI_CONFIGREG_SCLK_CTL_SHIFT;

    return 0;
}

int spi_xfer(spi_dev_t *dev, size_t len, uint8_t *data) {
    uint32_t reg;
    uint32_t burst_length;
    size_t transmitted_length = 0;
    size_t received_length = 0;
    calculated_reg_vals_t *reg_vals = (calculated_reg_vals_t*)&dev->internal[0];

    /* Logic won't work right on 0 length, and burst_length is limited to 2^7 (128) words which are 4 bytes each */
    if(len < 1 || len > 512) {
        return -1;
    }

    /* burst_length is the number of bits in the burst - 1 */
    burst_length = (len * 8) - 1;

    reg = in32(dev->base + MX6X_ECSPI_CONREG);
    reg &= ~(reg_vals->conreg_mask);
    reg |= reg_vals->conreg;
    reg &= ~(MX6X_ECSPI_CONREG_BURST_LENGTH_MASK);
    reg |= (burst_length << MX6X_ECSPI_CONREG_BURST_LENGTH_SHIFT) & MX6X_ECSPI_CONREG_BURST_LENGTH_MASK;
    out32(dev->base + MX6X_ECSPI_CONREG, reg);

    reg = in32(dev->base + MX6X_ECSPI_CONFIGREG);
    reg &= ~(reg_vals->configreg_mask);
    reg |= reg_vals->configreg;
    out32(dev->base + MX6X_ECSPI_CONFIGREG, reg);

    /* clean up the RXFIFO, there should be no data here */
    while(in32(dev->base + MX6X_ECSPI_TESTREG) & MX6X_ECSPI_TESTREG_RXCNT_MASK) {
        in32(dev->base + MX6X_ECSPI_RXDATA);
    }

    reg = 0;
    reg |= (RX_WATERMARK << MX6X_ECSPI_DMAREG_RX_THRESHOLD_SHIFT) & MX6X_ECSPI_DMAREG_RX_THRESHOLD_MASK;
    reg |= (TX_WATERMARK << MX6X_ECSPI_DMAREG_TX_THRESHOLD_SHIFT) & MX6X_ECSPI_DMAREG_TX_THRESHOLD_MASK;
    out32(dev->base + MX6X_ECSPI_DMAREG, reg);

    out32(dev->base + MX6X_ECSPI_STATREG, MX6X_ECSPI_STATREG_TC | MX6X_ECSPI_STATREG_RO);

    switch(len % 4) {
        case 1:
            out32(dev->base + MX6X_ECSPI_TXDATA, data[0]);
            transmitted_length += 1;
            break;

        case 2:
            out32(dev->base + MX6X_ECSPI_TXDATA, data[0] << 8 | data[1]);
            transmitted_length += 2;
            break;

        case 3:
            out32(dev->base + MX6X_ECSPI_TXDATA, data[0] << 16 | data[1] << 8 | data[2]);
            transmitted_length += 3;
            break;

        default:
            break;
    }

    while(transmitted_length < len) {
        while(in32(dev->base + MX6X_ECSPI_STATREG) & MX6X_ECSPI_STATREG_TF) {
            /* do nothing */
        }

        out32(dev->base + MX6X_ECSPI_TXDATA,
              data[transmitted_length + 0] << 24
            | data[transmitted_length + 1] << 16
            | data[transmitted_length + 2] << 8
            | data[transmitted_length + 3] << 0);
        transmitted_length += 4;

        if(in32(dev->base + MX6X_ECSPI_STATREG) & MX6X_ECSPI_STATREG_RDR) {
            size_t i;
            for(i = 0; i < RX_WATERMARK; i++) {
                received_length += read_word(dev, data, received_length, len);
            }
        }
    }

    while(!(in32(dev->base + MX6X_ECSPI_STATREG) & MX6X_ECSPI_STATREG_TC)) {
        /* do nothing */
    }

    while(in32(dev->base + MX6X_ECSPI_STATREG) & MX6X_ECSPI_STATREG_RR) {
        if(received_length < len) {
            received_length += read_word(dev, data, received_length, len);
        } else {
            in32(dev->base + MX6X_ECSPI_RXDATA); /* Got past the buffer length somehow, will go ahead and empty the fifo */
        }
    }

    out32(dev->base + MX6X_ECSPI_STATREG, MX6X_ECSPI_STATREG_TC | MX6X_ECSPI_STATREG_RO);

    if(received_length != transmitted_length) {
        return -1;
    }

    return 0;
}

static size_t read_word(spi_dev_t *dev, uint8_t *data, size_t received_length, size_t len) {
    uint32_t rec_word = in32(dev->base + MX6X_ECSPI_RXDATA);
    size_t   bytes_read = 0;

    if((received_length == 0) && ((len % 4) != 0)) {
        switch(len % 4) {
            case 1:
                data[received_length] = (uint8_t) rec_word;
                bytes_read += 1;
                break;

            case 2:
                data[received_length + 0] = (uint8_t) (rec_word >> 8);
                data[received_length + 1] = (uint8_t) (rec_word >> 0);
                bytes_read += 2;
                break;

            case 3:
                data[received_length + 0] = (uint8_t) (rec_word >> 16);
                data[received_length + 1] = (uint8_t) (rec_word >> 8);
                data[received_length + 2] = (uint8_t) (rec_word >> 0);
                bytes_read += 3;
                break;
        }
    } else {
        data[received_length + 0] = (uint8_t) (rec_word >> 24);
        data[received_length + 1] = (uint8_t) (rec_word >> 16);
        data[received_length + 2] = (uint8_t) (rec_word >> 8);
        data[received_length + 3] = (uint8_t) (rec_word >> 0);
        bytes_read += 4;

    }

    return bytes_read;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/mx6x_spi.c $ $Rev: 804201 $")
#endif
