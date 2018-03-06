/*
 * $QNXLicenseC:
 * Copyright 2013, QNX Software Systems.
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

#ifndef	_RCAR_SDHI_H_INCLUDED
#define	_RCAR_SDHI_H_INCLUDED

#include <internal.h>
#include <hw/dma.h>

#define SDHI_CHANNELS           4

#define IRQ_SDHI0               (165 + 32)
#define IRQ_SDHI1               (167 + 32)
#define IRQ_SDHI2               (168 + 32)

/*
 * SDIO/MMC Memory-mapped registers
 */

#define SDH_CMD                 0x00    // Command type register
#define SDH_PORTSEL             0x02    // SD port select register
#define SDH_ARG0                0x04    // SD command argument registers
#define SDH_ARG1                0x06
#define SDH_STOP                0x08    // Data stop register
#define SDH_SECCNT              0x0A    // Block count register
#define SDH_RSP00               0x0C    // SD card response registers
#define SDH_RSP01               0x0E
#define SDH_RSP02               0x10
#define SDH_RSP03               0x12
#define SDH_RSP04               0x14
#define SDH_RSP05               0x16
#define SDH_RSP06               0x18
#define SDH_RSP07               0x1A
#define SDH_INFO1               0x1C    // SD card interrupt flag register 1
#define SDH_INFO2               0x1E    // SD card interrupt flag register 2
#define SDH_INFO1_MASK          0x20    // SD_INFO1 interrupt mask register
#define SDH_INFO2_MASK          0x22    // SD_INFO2 interrupt mask register
#define SDH_CLK_CTRL            0x24    // SD clock control register
#define SDH_SIZE                0x26    // Transfer data length register
#define SDH_OPTION              0x28    // SD card access control option register
#define SDH_ERR_STS1            0x2C    // SD error status register 1
#define SDH_ERR_STS2            0x2E    // SD error status register 2
#define SDH_BUF0                0x30    // SD buffer read/write register
#define SDIO_MODE               0x34    // SDIO mode control register
#define SDIO_INFO1              0x36    // SDIO interrupt flag register
#define SDIO_INFO1_MASK         0x38    // SDIO_INFO1 interrupt mask register
#define SDH_CC_EXT_MODE         0xD8    // DMA mode enable register
#define SDH_SOFT_RST            0xE0    // Software reset register
#define SDH_VERSION             0xE2    // Version register
#define SDH_EXT_ACC             0xE4
#define SDH_EXT_SWAP            0xF0    // Swap control register
#define SDH_DMACR_CB0D          0x192
#define SDH_DMACR_490C          0xE6

/* SCC registers */
#define SDH_SCC_DTCNTL          0x300
#define SDH_SCC_TAPSET          0x304
#define SDH_SCC_DT2FF           0x308
#define SDH_SCC_CKSEL           0x30C
#define SDH_SCC_RVSCNTL         0x310
#define SDH_SCC_RVSREQ          0x314

// Command register bits
#define SDH_CMD_AC12            (0 << 14)   // CMD12 is automatically issued
#define SDH_CMD_NOAC12          (1 << 14)
#define SDH_CMD_DAT_MULTI       (1 << 13)   // multi block transfer
#define SDH_CMD_DAT_READ        (1 << 12)   // read
#define SDH_CMD_ADTC            (1 << 11)   // command with data
#define SDH_CMD_NORSP           (3 <<  8)   // no response
#define SDH_CMD_RSPR1           (4 <<  8)   // R1, R5, R6, R7
#define SDH_CMD_RSPR1B          (5 <<  8)   // R1b
#define SDH_CMD_RSPR2           (6 <<  8)   // R2
#define SDH_CMD_RSPR3           (7 <<  8)   // R3, R4
#define SDH_CMD_ACMD            (6 <<  8)   // ACMD

// Stop register
#define SDH_STOP_STP            (1 <<  0)
#define SDH_STOP_SEC            (1 <<  8)

// CLK_CTRL register
#define SDH_CLKCTRL_SCLKEN      (1 <<  8)

// INFO1
#define SDH_INFO1_WP            (1 <<  7)   // write protect
#define SDH_INFO1_CD            (1 <<  5)   // card detection state
#define SDH_INFO1_INST          (1 <<  4)   // card insertion
#define SDH_INFO1_RMVL          (1 <<  3)   // card removal
#define SDH_INFO1_AE            (1 <<  2)   // access end
#define SDH_INFO1_RE            (1 <<  0)   // response end

// INFO2
#define SDH_INFO2_ILA           (1 << 15)   // illegal access error
#define SDH_INFO2_CBSY          (1 << 14)   // command response busy
#define SDH_INFO2_SCLKDIVEN     (1 << 13)   // SD bus busy
#define SDH_INFO2_BWE           (1 <<  9)   // SD_BUF Write Enable
#define SDH_INFO2_BRE           (1 <<  8)   // SD_BUF Read Enable
#define SDH_INFO2_RTO           (1 <<  6)   // Response Timeout
#define SDH_INFO2_BIRA          (1 <<  5)   // SD_BUF Illegal Read Access
#define SDH_INFO2_BIWA          (1 <<  4)   // SD_BUF Illegal Write Access
#define SDH_INFO2_DTO           (1 <<  3)   // Timeout (except response timeout)
#define SDH_INFO2_ENDE          (1 <<  2)   // END Error
#define SDH_INFO2_CRCE          (1 <<  1)   // CRC Error
#define SDH_INFO2_CMDE          (1 <<  0)   // CMD Error
#define SDH_INFO2_ALL_ERR       (SDH_INFO2_CMDE | SDH_INFO2_CRCE | SDH_INFO2_ENDE | SDH_INFO2_DTO |    \
                                                  SDH_INFO2_BIWA | SDH_INFO2_BIRA | SDH_INFO2_RTO)

// SD Card Access Control Option Register (SD_OPTION) bit defination
#define SDH_OPTION_WIDTH_1      (1 << 15)   // Data Bus Width 1 bit
#define SDH_OPTION_WIDTH_4      (0 << 15)   // Data Bus Width 4 bit

// Revision register
#define RCAR_SDHI_VER_490C      0x490C
#define RCAR_SDHI_VER_CB0D      0xCB0D

// DMA Mode Enable Register (CC_EXT_MODE) bit defination
#define SDH_CC_EXT_DMASDRW      (1 <<  1)   // The SD_BUF read/write DMA transfer is enabled

/* Definitions for values the RCAR_SDHI_SCC_DTCNTL register */
#define RCAR_SDHI_SCC_DTCNTL_TAPEN      (1 << 0)
/* Definitions for values the RCAR_SDHI_SCC_CKSEL register */
#define RCAR_SDHI_SCC_CKSEL_DTSEL       (1 << 0)
/* Definitions for values the RCAR_SDHI_SCC_RVSCNTL register */
#define RCAR_SDHI_SCC_RVSCNTL_RVSEN     (1 << 0)
/* Definitions for values the RCAR_SDHI_SCC_RVSREQ register */
#define RCAR_SDHI_SCC_RVSREQ_RVSERR     (1 << 2)

#define RCAR_SDHI_HAS_UHS_SCC       1

#define RCAR_SDHI_MAX_TAP           3

/* Maximum number DMA transfer size */
#define RCAR_SDHI_DMA_XMIT_SZ_MAX   6

#define SDHI_TMOUT                  1000000
#define RCAR_SDHI_TUNING_TIMEOUT    150
#define RCAR_SDHI_TUNING_RETRIES    40

/* Product device */
#define PRODUCT_REGISTER    0xFF000044
#define PRODUCT_CUT_MASK    0x00007FF0
#define PRODUCT_H2_BIT      (0x45 << 8)

typedef struct _sdhi_scc_t {
    uint32_t    clk;    /* clock for SDR104 */
    uint32_t    tap;    /* sampling clock position for SDR104 */
} sdhi_scc_t;

typedef struct _sdhi_cfg_t
{
    uint32_t    base;
    uint8_t     tx_mid_rid;
    uint8_t     rx_mid_rid;
    uint32_t    clock;
    uint16_t    irq;
    uint8_t     bus_shift;
    sdhi_scc_t  *taps;
    int         taps_num;
} sdhi_cfg_t;

typedef	struct _rcar_sdhi_t {
    void            *bshdl;
    uint32_t        flags;
    uint32_t        pclk;
    int             bus_shift;
    int             card_insert;    // This is to avoid insert and then remove interrupt
    uint32_t        busclk;

    int             irq;
    paddr_t         pbase;
    uintptr_t       base;
    uint8_t         tx_mid_rid;
    uint8_t         rx_mid_rid;
    uint16_t        rev;            // Revision

    uint16_t        blksz;
    uint16_t        mask1;
    uint16_t        mask2;

    dma_functions_t dmafuncs;
    void            *dma;
#define DMA_DESC_MAX    256
    sdio_sge_t      sgl[DMA_DESC_MAX];
} rcar_sdhi_t;

static inline uint16_t rcar_sdh_read(rcar_sdhi_t *sdhi, int reg)
{
    return in16(sdhi->base + (reg << sdhi->bus_shift));
}

static inline void rcar_sdh_write(rcar_sdhi_t *sdhi, int reg, uint16_t val)
{
    out16(sdhi->base + (reg << sdhi->bus_shift), val);
}

extern int rcar_sdhi_init(sdio_hc_t *hc);
extern int rcar_sdhi_dinit(sdio_hc_t *hc);

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/devb/sdmmc/sdiodi/hc/rcar_sdhi.h $ $Rev: 798452 $")
#endif
