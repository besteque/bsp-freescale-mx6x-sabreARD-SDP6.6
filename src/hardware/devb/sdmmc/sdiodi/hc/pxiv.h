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

#ifndef _PXIV_H_INCLUDED
#define _PXIV_H_INCLUDED

#include <internal.h>

#define PXIV_SDHC_SIZE              0x500
#define PXIV_SDHC0_BASE             0x5a000000
#define PXIV_SDHC1_BASE             0x5a400000

#define PXIV_SDHC_CTL_SIZE          0x200
#define PXIV_SDHC0_CTL_BASE         0x59810200
#define PXIV_SDHC1_CTL_BASE         0x59810000

#define PXIV_SDHC0_IRQ              110
#define PXIV_SDHC1_IRQ              108

// eMMC/SD interface control registers
#define PXIV_SD_CLKCTRL             0x0020
    #define PXIV_SD_CLKCTRL_SDIO_ON             (1 << 8)

#define PXIV_SD_MODECTRL            0x0030
    #define PXIV_SD_MODECTRL_MMCMODE            (1 << 27)   // 0 = SD, 1 = eMMC
    #define PXIV_SD_MODECTRL_BSWAPEN            (1 << 21)
    #define PXIV_SD_MODECTRL_CLKSEL0_SHIFT      16          // 0 = 44.4, 1 = 33.3, 2 = 50, 3 = 66.7 [MHz]
    #define PXIV_SD_MODECTRL_CLKSEL0_MASK       (0x3 << PXIV_SD_MODECTRL_CLKSEL0_SHIFT)
    #define PXIV_SD_MODECTRL_CLKMODE            (1 << 12)   // 1 = CLKSEL1 bits are valid, 0 = CLKSEL0 (I/F clk freq)
    #define PXIV_SD_MODECTRL_CLKSEL1_SHIFT      8           // 0 = 100, 1 = 40, 2 = 25, 3 = 22.2 [MHz]
    #define PXIV_SD_MODECTRL_CLKSEL1_MASK       (0x3 << PXIV_SD_MODECTRL_CLKSEL1_SHIFT)

#define PXIV_SD_BYTESWAP            0x00CC
#define PXIV_SD_RSTCTRL             0x0110
    #define PXIV_SD_RSTCTRL_SDIORST             (1 << 0)    // Clear reset

#define PXIV_SD_EMMC1_HWRST         0x0080

// DMA registers
#define PXIV_DM_CM_DTRAN_MODE       0x0410
    #define PXIV_DM_CM_DTRAN_MODE_ADDR_MODE     (1 << 0)    // 0 = Fixed address, 1 = increment
    #define PXIV_DM_CM_DTRAN_MODE_WIDTH_SHIFT   4
    #define PXIV_DM_CM_DTRAN_MODE_WIDTH_MASK    (0x3 < PXIV_DM_CM_DTRAN_MODE_BUS_WIDTH_SHIFT)
    #define PXIV_DM_CM_DTRAN_MODE_WIDTH_64      (3 << PXIV_DM_CM_DTRAN_MODE_WIDTH_SHIFT)
    #define PXIV_DM_CM_DTRAN_MODE_WIDTH_32      (2 << PXIV_DM_CM_DTRAN_MODE_WIDTH_SHIFT)
    #define PXIV_DM_CM_DTRAN_MODE_WIDTH_16      (1 << PXIV_DM_CM_DTRAN_MODE_WIDTH_SHIFT)
    #define PXIV_DM_CM_DTRAN_MODE_WIDTH_8       (0 << PXIV_DM_CM_DTRAN_MODE_WIDTH_SHIFT)
    #define PXIV_DM_CM_DTRAN_MODE_CH_SHIFT      16
    #define PXIV_DM_CM_DTRAN_MODE_CH_MASK       (0x3 << PXIV_DM_CM_DTRAN_MODE_CH_SHIFT)
    #define PXIV_DM_CM_DTRAN_MODE_CH_TX         (0 << PXIV_DM_CM_DTRAN_MODE_CH_SHIFT)
    #define PXIV_DM_CM_DTRAN_MODE_CH_RX         (1 << PXIV_DM_CM_DTRAN_MODE_CH_SHIFT)

#define PXIV_DM_CM_DTRAN_CTRL       0x0414
    #define PXIV_DM_CM_DTRAN_CTRL_DM_START      (1 << 0)

#define PXIV_DM_CM_RST              0x0418
    #define PXIV_DM_CM_RST_DTRANRST_SHIFT       8           // Reset each DMA channel (on error bit in DM_INFO2)
    #define PXIV_DM_CM_RST_DTRANRST_MASK        (0x3 << PXIV_DM_CM_RST_DTRANRST_SHIFT)

#define PXIV_DM_CM_INFO1            0x0420                  // Interrupt register for DMA
    #define PXIV_DM_CM_INFO1_DTAREND_SHIFT      16
    #define PXIV_DM_CM_INFO1_DTAREND_MASK       (0x1F << PXIV_DM_CM_INFO1_DTAREND_SHIFT)
    #define PXIV_DM_CM_INFO1_DTAREND_TX         (0x1 << PXIV_DM_CM_INFO1_DTAREND_SHIFT)
    #define PXIV_DM_CM_INFO1_DTAREND_RX         (0x2 << PXIV_DM_CM_INFO1_DTAREND_SHIFT)

#define PXIV_DM_CM_INFO1_MASK       0x0424
    #define PXIV_DM_CM_INFO1_MASK_DTAREND_SHIFT 16
    #define PXIV_DM_CM_INFO1_MASK_DTAREND_MASK  (0x1F << PXIV_DM_CM_INFO1_MASK_DTAREND_SHIFT)
    #define PXIV_DM_CM_INFO1_MASK_DTAREND_TX    (0x1 << PXIV_DM_CM_INFO1_MASK_DTAREND_SHIFT)
    #define PXIV_DM_CM_INFO1_MASK_DTAREND_RX    (0x2 << PXIV_DM_CM_INFO1_MASK_DTAREND_SHIFT)
    #define PXIV_DM_CM_INFO1_MASK_ALL           0xFFFFFFFF

#define PXIV_DM_CM_INFO2            0x0428                  // Error register for DMA
    #define PXIV_DM_CM_INFO2_DTARERR_SHIFT      16
    #define PXIV_DM_CM_INFO2_DTARERR_MASK       (0xF << PXIV_DM_CM_INFO2_DTARERR_SHIFT)
    #define PXIV_DM_CM_INFO2_DTARERR_TX         (0x1 << PXIV_DM_CM_INFO2_DTARERR_SHIFT)
    #define PXIV_DM_CM_INFO2_DTARERR_RX         (0x2 << PXIV_DM_CM_INFO2_DTARERR_SHIFT)
    #define PXIV_DM_CM_INFO2_MASK_ALL           0xFFFFFFFF

#define PXIV_DM_CM_INFO2_MASK       0x042C
    #define PXIV_DM_CM_INFO2_MASK_DTARERR_SHIFT 16
    #define PXIV_DM_CM_INFO2_MASK_DTARERR_MASK  (0xF << PXIV_DM_CM_INFO2_MASK_DTARERR_SHIFT)
    #define PXIV_DM_CM_INFO2_MASK_DTARERR_TX    (0x1 << PXIV_DM_CM_INFO2_MASK_DTARERR_SHIFT)
    #define PXIV_DM_CM_INFO2_MASK_DTARERR_RX    (0x2 << PXIV_DM_CM_INFO2_MASK_DTARERR_SHIFT)

#define PXIV_DM_DTRAN_ADDR          0x0440
#define PXIV_DM_DTRAN_ADDREX        0x0444

// HC registers
#define PXIV_SD_CMD                 0x0000
    #define     PXIV_SD_CMD_MD7_SHIFT           14          // Auto CMD12
    #define     PXIV_SD_CMD_MD7_MASK            (0x3 << PXIV_SD_CMD_MD7_6_SHIFT)
    #define     PXIV_SD_CMD_MD5_BLK_MULT        (1 << 13)   // Multi block transfer
    #define     PXIV_SD_CMD_MD4_READ            (1 << 12)   // IN (read) transfer
    #define     PXIV_SD_CMD_MD3_ADTC            (1 << 11)   // ADTC
    #define     PXIV_SD_CMD_MD2_SHIFT           8           // Extended command mode (for eMMC)
    #define     PXIV_SD_CMD_MD2_MASK            (0x7 << PXIV_SD_CMD_MD2_SHIFT)
    #define     PXIV_SD_CMD_MD2_NORMAL_MODE     (0x0 << PXIV_SD_CMD_MD2_SHIFT)
    #define     PXIV_SD_CMD_MD2_EXT_NO_RESP     (0x3 << PXIV_SD_CMD_MD2_SHIFT)
    #define     PXIV_SD_CMD_MD2_EXT_RESP_R1     (0x4 << PXIV_SD_CMD_MD2_SHIFT)
    #define     PXIV_SD_CMD_MD2_EXT_RESP_R1B    (0x5 << PXIV_SD_CMD_MD2_SHIFT)
    #define     PXIV_SD_CMD_MD2_EXT_RESP_R2     (0x6 << PXIV_SD_CMD_MD2_SHIFT)
    #define     PXIV_SD_CMD_MD2_EXT_RESP_R3     (0x7 << PXIV_SD_CMD_MD2_SHIFT)
    #define     PXIV_SD_CMD_C_SHIFT             6           // Command type
    #define     PXIV_SD_CMD_C_MASK              (0x3 << PXIV_SD_CMD_C_SHIFT)
    #define     PXIV_SD_CMD_C_APP_ACMD          (0x1 << PXIV_SD_CMD_C_SHIFT)
    #define     PXIV_SD_CMD_CF_SHIFT            0           // Command index
    #define     PXIV_SD_CMD_CF_MASK             (0x3F << PXIV_SD_CMD_CF_SHIFT)

#define PXIV_SD_ARG0                0x0008
#define PXIV_SD_STOP                0x0010
    #define PXIV_SD_STOP_SEC                    (1 << 8)    // Enable SD_SECCNT

#define PXIV_SD_SECCNT              0x0014
#define PXIV_SD_RSP10               0x0018
#define PXIV_SD_RSP32               0x0020
#define PXIV_SD_RSP54               0x0028
#define PXIV_SD_RSP76               0x0030
#define PXIV_SD_INFO1               0x0038                  // Card status register (writing 0 clears R/W bits)
    #define PXIV_SD_INFO1_HPIRES                (1 << 16)   // Response receipt interrupt in HPI mode
    #define PXIV_SD_INFO1_INFO10                (1 << 10)   // SDDAT3 CD
    #define PXIV_SD_INFO1_INFO9                 (1 << 9)    // SDDAT3 card insertion
    #define PXIV_SD_INFO1_INFO8                 (1 << 8)    // SDDAT3 card removal
    #define PXIV_SD_INFO1_INFO7_WP              (1 << 7)    // WP
    #define PXIV_SD_INFO1_INFO5_CD              (1 << 5)    // CD
    #define PXIV_SD_INFO1_INFO4_INSERT          (1 << 4)    // Card insertion
    #define PXIV_SD_INFO1_INFO3_REMOVE          (1 << 3)    // Card removal
    #define PXIV_SD_INFO1_INFO2_TC              (1 << 2)    // Transfer complete
    #define PXIV_SD_INFO1_INFO0_CC              (1 << 0)    // CC
    #define PXIV_SD_INFO1_ALL_CMP               (PXIV_SD_INFO1_INFO2_TC | PXIV_SD_INFO1_INFO0_CC)

#define PXIV_SD_INFO2               0x003C                  // Buffer control and error interrupt register (writing 0 clears R/W bits)
    #define PXIV_SD_INFO2_ILA                   (1 << 15)   // Illegal access error
    #define PXIV_SD_INFO2_CBUSY                 (1 << 14)   // Command busy
    #define PXIV_SD_INFO2_SCLKDIVEN             (1 << 13)   // CMD / DATA lines are not busy
    #define PXIV_SD_INFO2_BWE                   (1 << 9)    // Buffer write enable
    #define PXIV_SD_INFO2_BRE                   (1 << 8)    // Buffer read enable
    #define PXIV_SD_INFO2_DATA0                 (1 << 7)    // SDDATA0 status
    #define PXIV_SD_INFO2_ERR6_RTO              (1 << 6)    // Command response timeout
    #define PXIV_SD_INFO2_ERR5_ILRA             (1 << 5)    // Illegal read access
    #define PXIV_SD_INFO2_ERR4_ILWA             (1 << 4)    // Illegal write access
    #define PXIV_SD_INFO2_ERR3_DTO              (1 << 3)    // Data timeout error
    #define PXIV_SD_INFO2_ERR2_ENDBIT           (1 << 2)    // End bit error
    #define PXIV_SD_INFO2_ERR1_CRC              (1 << 1)    // CRC bit error
    #define PXIV_SD_INFO2_ERR0_CMD              (1 << 0)    // Command index error
    #define PXIV_SD_INFO2_ALL_ERR               (PXIV_SD_INFO2_ILA          | \
                                                 PXIV_SD_INFO2_ERR6_RTO     | \
                                                 PXIV_SD_INFO2_ERR5_ILRA    | \
                                                 PXIV_SD_INFO2_ERR4_ILWA    | \
                                                 PXIV_SD_INFO2_ERR3_DTO     | \
                                                 PXIV_SD_INFO2_ERR2_ENDBIT  | \
                                                 PXIV_SD_INFO2_ERR1_CRC     | \
                                                 PXIV_SD_INFO2_ERR0_CMD)

#define PXIV_SD_INFO1_MASK          0x0040
    #define PXIV_SD_INFO1_IMASK16               (1 << 16)
    #define PXIV_SD_INFO1_IMASK9                (1 << 9)
    #define PXIV_SD_INFO1_IMASK8                (1 << 8)
    #define PXIV_SD_INFO1_IMASK4                (1 << 4)
    #define PXIV_SD_INFO1_IMASK3                (1 << 3)
    #define PXIV_SD_INFO1_IMASK2                (1 << 2)
    #define PXIV_SD_INFO1_IMASK0                (1 << 0)
    #define PXIV_SD_INFO1_MASK_ALL              0x1031D

#define PXIV_SD_INFO2_MASK          0x0044
    #define PXIV_SD_INFO2_IMASK                 (1 << 15)
    #define PXIV_SD_INFO2_BMASK1                (1 << 9)
    #define PXIV_SD_INFO2_BMASK0                (1 << 8)
    #define PXIV_SD_INFO2_EMASK6                (1 << 6)
    #define PXIV_SD_INFO2_EMASK5                (1 << 5)
    #define PXIV_SD_INFO2_EMASK4                (1 << 4)
    #define PXIV_SD_INFO2_EMASK3                (1 << 3)
    #define PXIV_SD_INFO2_EMASK2                (1 << 2)
    #define PXIV_SD_INFO2_EMASK1                (1 << 1)
    #define PXIV_SD_INFO2_EMASK0                (1 << 0)
    #define PXIV_SD_INFO2_MASK_ALL              0x8B7F      // Reserved bits (e.g. bit 11) have to be written with orig values

#define PXIV_SD_CLK_CTRL            0x0048
    #define PXIV_SD_CLK_CTRL_DIV8               (1 << 16)
    #define PXIV_SD_CLK_CTRL_SDCLKSEL           (1 << 10)   // HS / SDR25 / SDR50 mode
    #define PXIV_SD_CLK_CTRL_SDCLKOFFEN         (1 << 9)    // CLK auto stop
    #define PXIV_SD_CLK_CTRL_SCLKEN             (1 << 8)
    #define PXIV_SD_CLK_CTRL_DIV_SHIFT          0
    #define PXIV_SD_CLK_CTRL_DIV_MSK            (0xFF << PXIV_SD_CLK_CTRL_DIV_SHIFT)

#define PXIV_SD_SIZE                0x004C
#define PXIV_SD_OPTION              0x0050
    #define PXIV_SD_OPTION_WIDTH                (1 << 15)
    #define PXIV_SD_OPTION_WIDTH8               (1 << 13)
    #define PXIV_SD_OPTION_EXTOP                (1 << 9)
    #define PXIV_SD_OPTION_TOUTMASK             (1 << 8)

#define PXIV_SD_ERR_STS1            0x0058
    #define PXIV_SD_ERR_STS1_E11                (1 << 11)   // Err in CRC status
    #define PXIV_SD_ERR_STS1_E10                (1 << 10)   // CRC err in read data
    #define PXIV_SD_ERR_STS1_E9                 (1 << 9)    // CRC err in cmd resp STP / HPI / abort
    #define PXIV_SD_ERR_STS1_E8                 (1 << 8)    // CRC err in cmd resp
    #define PXIV_SD_ERR_STS1_E5                 (1 << 5)    // End bit err in CRC status
    #define PXIV_SD_ERR_STS1_E4                 (1 << 4)    // End bit err in read data
    #define PXIV_SD_ERR_STS1_E3                 (1 << 3)    // End but err in cmd resp to STP / HPI / abort
    #define PXIV_SD_ERR_STS1_E2                 (1 << 2)    // End bit err in cmd resp
    #define PXIV_SD_ERR_STS1_E1                 (1 << 1)    // CMD index err in cmd resp to STP / HPI / abort
    #define PXIV_SD_ERR_STS1_E0                 (1 << 0)    // CMD index err in resp

#define PXIV_SD_ERR_STS2            0x005C
    #define PXIV_SD_ERR_STS2_E6                 (1 << 6)    // Busy TO (after CRC status received)
    #define PXIV_SD_ERR_STS2_E5                 (1 << 5)    // CRC status TO
    #define PXIV_SD_ERR_STS2_E4                 (1 << 4)    // Read data TO
    #define PXIV_SD_ERR_STS2_E3                 (1 << 3)    // Busy TO (STP / abort)
    #define PXIV_SD_ERR_STS2_E2                 (1 << 2)    // Busy TO after resp
    #define PXIV_SD_ERR_STS2_E1                 (1 << 1)    // RTO (STP / abort)
    #define PXIV_SD_ERR_STS2_E0                 (1 << 0)    // RTO

#define PXIV_SD_BUF                 0x0060
#define PXIV_SDIO_MODE              0x0068
#define PXIV_SDIO_INFO1             0x006C
#define PXIV_SDIO_INFO1_MASK        0x0070
#define PXIV_CC_EXT_MODE            0x01B0
    #define PXIV_CC_EXT_MODE_DMASDRW            (1 << 1)    // Enable DMA mode

#define PXIV_SOFT_RST               0x01C0
    #define PXIV_SOFT_RST_SDRST                 (1 << 0)

#define PXIV_VERSION                0x01C4
#define PXIV_HOST_MODE              0x01C8
    #define PXIV_HOST_MODE_BUSWIDTH             (1 << 8)    // SD buffer access width (0 = 16bit, 1 = 32bit)
    #define PXIV_HOST_MODE_WMODE                (1 << 0)    // SD buffer access control enable (0 = 64bit, 1 = value from bit 8)

#define PXIV_SDIF_MODE              0x01CC
    #define PXIV_SDIF_MODE_DDR                  (1 << 0)    // 1 = enable DDR50

#define PXIV_SD_STATUS              0x01E4
    #define PXIV_SD_STATUS_PWR_OFF              0x00
    #define PXIV_SD_STATUS_PWR_3V3              0x01
    #define PXIV_SD_STATUS_PWR_1V8              0x02
    #define PXIV_SD_STATUS_PWR_MSK              0x03

#define PXIV_EXT_SDIO               0x01E8

#define PXIV_CLOCK_DEFAULT          50000000                // Interface clock = 100 MHz and if using the CLK divider (non-SDR50
                                                            // modes) the minimal divider is 2

#define PXIV_IF_RESET               (1 << 0)                // Interface reset control register
#define PXIV_HC_SOFT_RST            (1 << 1)                // Softreset register (error recovery during transaction)
#define PXIV_RESET_ALL              (PXIV_IF_RESET | PXIV_HC_SOFT_RST)

#define PXIV_CLOCK_TIMEOUT          10000
#define PXIV_COMMAND_TIMEOUT        1000000                 // The card could take very long time to process the transfered data
#define PXIV_TRANSFER_TIMEOUT       1000000


typedef struct _pxiv_sdmmc_hc {
    void            *bshdl;
    uintptr_t       ctrl_base;      // mode and clock control registers
    uintptr_t       base;           // HC registers
#define SF_USE_SDMA     0x01
#define SF_SDMA_ACTIVE  0x02
    uint32_t        flags;
#define DMA_DESC_MAX    64
    sdio_sge_t      sgl[DMA_DESC_MAX];
} pxiv_sdmmc_hc_t;

extern int pxiv_init( sdio_hc_t *hc );
extern int pxiv_dinit( sdio_hc_t *hc );

#endif  /* _PXIV_H_INCLUDED */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/devb/sdmmc/sdiodi/hc/pxiv.h $ $Rev: 793476 $")
#endif
