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

#ifndef _HSMCI_H_INCLUDED
#define _HSMCI_H_INCLUDED

#define ATMEL_HSMCI_MCK                 133000000

/*** HSMCI register definitions ***/
#define ATMEL_HSMCI_SIZE                (0x600)

/* HSMCI Control Register */
#define ATMEL_HSMCI_CR                  (0x000)
#define ATMEL_HSMCI_CR_MCIEN            (1 << 0)
#define ATMEL_HSMCI_CR_MCIDIS           (1 << 1)
#define ATMEL_HSMCI_CR_PWSEN            (1 << 2)
#define ATMEL_HSMCI_CR_PWSDIS           (1 << 3)
#define ATMEL_HSMCI_CR_SWRST            (1 << 7)

/* HSMCI Mode Register */
#define ATMEL_HSMCI_MR                  (0x004)
#define ATMEL_HSMCI_MR_CLKDIV_MASK      (0xFF)
#define ATMEL_HSMCI_MR_CLKDIV(_d_)      ((_d_) & ATMEL_HSMCI_MR_CLKDIV_MASK)
#define ATMEL_HSMCI_MR_PWSDIV_MAX       (0x7 << 8)
#define ATMEL_HSMCI_MR_RDPROOF          (1 << 11)
#define ATMEL_HSMCI_MR_WRPROOF          (1 << 12)
#define ATMEL_HSMCI_MR_FBYTE            (1 << 13)
#define ATMEL_HSMCI_MR_PADV             (1 << 14)
#define ATMEL_HSMCI_MR_CLKODD           (1 << 16)

/* HSMCI Data Timeout Register */
#define ATMEL_HSMCI_DTOR                (0x008)
#define ATMEL_HSMCI_DTOR_MAX            (0x7F)

/* HSMCI SDCard/SDIO Register */
#define ATMEL_HSMCI_SDCR                (0x00C)
#define ATMEL_HSMCI_SDCR_SDCSEL_SLOTA   (0 << 0)
#define ATMEL_HSMCI_SDCR_SDCSEL_SLOTB   (1 << 0)
#define ATMEL_HSMCI_SDCR_SDCSEL_SLOTC   (2 << 0)
#define ATMEL_HSMCI_SDCR_SDCSEL_SLOTD   (3 << 0)
#define ATMEL_HSMCI_SDCR_SDCBUS_BW1     (0 << 6)
#define ATMEL_HSMCI_SDCR_SDCBUS_BW4     (2 << 6)
#define ATMEL_HSMCI_SDCR_SDCBUS_BW8     (3 << 6)

/* HSMCI Argument Register */
#define ATMEL_HSMCI_ARGR                (0x010)

/* HSMCI Command Register */
#define ATMEL_HSMCI_CMDR                (0x014)
#define ATMEL_HSMCI_CMDR_CMDNB_MASK     (0x3F)
#define ATMEL_HSMCI_CMDR_CMDNB(_c_)     ((_c_) & ATMEL_HSMCI_CMDR_CMDNB_MASK)
#define ATMEL_HSMCI_CMDR_RSPTYP_NORESP  (0 << 6)
#define ATMEL_HSMCI_CMDR_RSPTYP_48      (1 << 6)
#define ATMEL_HSMCI_CMDR_RSPTYP_136     (2 << 6)
#define ATMEL_HSMCI_CMDR_RSPTYP_R1B     (3 << 6)
#define ATMEL_HSMCI_CMDR_SPCMD_STD      (0 << 8)
#define ATMEL_HSMCI_CMDR_SPCMD_INIT     (1 << 8)
#define ATMEL_HSMCI_CMDR_SPCMD_SYNC     (2 << 8)
#define ATMEL_HSMCI_CMDR_SPCMD_CE_ATA   (3 << 8)
#define ATMEL_HSMCI_CMDR_SPCMD_IT_CMD   (4 << 8)
#define ATMEL_HSMCI_CMDR_SPCMD_IT_RESP  (5 << 8)
#define ATMEL_HSMCI_CMDR_SPCMD_BOR      (6 << 8)
#define ATMEL_HSMCI_CMDR_SPCMD_EBO      (7 << 8)
#define ATMEL_HSMCI_CMDR_OPCMD          (1 << 11)
#define ATMEL_HSMCI_CMDR_MAXLAT5        (0 << 12)
#define ATMEL_HSMCI_CMDR_MAXLAT64       (1 << 12)
#define ATMEL_HSMCI_CMDR_TRCMD_NO_DATA  (0 << 16)
#define ATMEL_HSMCI_CMDR_TRCMD_START    (1 << 16)
#define ATMEL_HSMCI_CMDR_TRCMD_STOP     (2 << 16)
#define ATMEL_HSMCI_CMDR_TDIR_WRITE     (0 << 18)
#define ATMEL_HSMCI_CMDR_TDIR_READ      (1 << 18)
#define ATMEL_HSMCI_CMDR_TRTYP_SINGLE   (0 << 19)
#define ATMEL_HSMCI_CMDR_TRTYP_MULTI    (1 << 19)
#define ATMEL_HSMCI_CMDR_TRTYP_STREAM   (2 << 19)
#define ATMEL_HSMCI_CMDR_TRTYP_BYTE     (4 << 19)
#define ATMEL_HSMCI_CMDR_TRTYP_BLOCK    (5 << 19)
#define ATMEL_HSMCI_CMDR_BOOT_ACK       (1 << 27)

/* HSMCI Block Register */
#define ATMEL_HSMCI_BLKR                (0x018)
#define ATMEL_HSMCI_BLKR_BLKLEN(_n_)    ((_n_) << 16)
#define ATMEL_HSMCI_BLKR_BCNT(_n_)      ((_n_) & 0xFF)

/* HSMCI Response Register */
#define ATMEL_HSMCI_RSPR                (0x020)

/* HSMCI Receive Data Register */
#define ATMEL_HSMCI_RDR                 (0x030)

/* HSMCI Transmit Data Register */
#define ATMEL_HSMCI_TDR                 (0x034)

/* HSMCI Status Register */
#define ATMEL_HSMCI_SR                  (0x040)
#define ATMEL_HSMCI_SR_CMDRDY           (1 << 0)
#define ATMEL_HSMCI_SR_RXRDY            (1 << 1)
#define ATMEL_HSMCI_SR_TXRDY            (1 << 2)
#define ATMEL_HSMCI_SR_BLKE             (1 << 3)
#define ATMEL_HSMCI_SR_DTIP             (1 << 4)
#define ATMEL_HSMCI_SR_NOTBUSY          (1 << 5)
#define ATMEL_HSMCI_SR_SDIOIRQA         (1 << 8)
#define ATMEL_HSMCI_SR_SDIOWAIT         (1 << 12)
#define ATMEL_HSMCI_SR_CSRCV            (1 << 13)
#define ATMEL_HSMCI_SR_RINDE            (1 << 16)
#define ATMEL_HSMCI_SR_RDIRE            (1 << 17)
#define ATMEL_HSMCI_SR_RCRCE            (1 << 18)
#define ATMEL_HSMCI_SR_RENDE            (1 << 19)
#define ATMEL_HSMCI_SR_RTOE             (1 << 20)
#define ATMEL_HSMCI_SR_DCRCE            (1 << 21)
#define ATMEL_HSMCI_SR_DTOE             (1 << 22)
#define ATMEL_HSMCI_SR_CSTOE            (1 << 23)
#define ATMEL_HSMCI_SR_BLKOVRE          (1 << 24)
#define ATMEL_HSMCI_SR_DMADONE          (1 << 25)
#define ATMEL_HSMCI_SR_FIFOEMPTY        (1 << 26)
#define ATMEL_HSMCI_SR_XFRDONE          (1 << 27)
#define ATMEL_HSMCI_SR_ACKRCVE          (1 << 29)
#define ATMEL_HSMCI_SR_OVRE             (1 << 30)
#define ATMEL_HSMCI_SR_UNRE             (1 << 31)
#define ATMEL_HSMCI_SR_OERRORS          (ATMEL_HSMCI_SR_RDIRE   | \
                                            ATMEL_HSMCI_SR_BLKOVRE | \
                                            ATMEL_HSMCI_SR_ACKRCVE)

/* HSMCI Interrupt Enable Register */
#define ATMEL_HSMCI_IER                 (0x044)
/* Same bits as ATMEL_HSMCI_SR */

/* HSMCI Interrupt Disable Register */
#define ATMEL_HSMCI_IDR                 (0x048)
/* Same bits as ATMEL_HSMCI_SR */

/* HSMCI Interrupt Mask Register */
#define ATMEL_HSMCI_IMR                 (0x04C)
/* Same bits as ATMEL_HSMCI_SR */

/* HSMCI DMA Configuration Register */
#define ATMEL_HSMCI_DMA                 (0x050)
#define ATMEL_HSMCI_DMA_OFFSET_0        (0)
#define ATMEL_HSMCI_DMA_CHKSIZE_1       (0 << 4)
#define ATMEL_HSMCI_DMA_CHKSIZE_4       (1 << 4)
#define ATMEL_HSMCI_DMA_CHKSIZE_8       (2 << 4)
#define ATMEL_HSMCI_DMA_CHKSIZE_16      (3 << 4)
#define ATMEL_HSMCI_DMA_CHKSIZE_32      (4 << 4)
#define ATMEL_HSMCI_DMA_DMAEN           (1 << 8)
#define ATMEL_HSMCI_DMA_ROPT            (1 << 12)

/* HSMCI Configuration Register */
#define ATMEL_HSMCI_CFG                 (0x04C)
#define ATMEL_HSMCI_CFG_HSMODE          (1 << 8)

/* HSMCI FIFOx Memory Aperture */
#define ATMEL_HSMCIO_FIFO               (0x200)

#define ATMEL_AHB_IF0                   0
#define ATMEL_AHB_IF1                   1
#define ATMEL_AHB_IF2                   2

/* Multi Buffer Transfer Using Linked List */
typedef struct {
    uint32_t src;
    uint32_t dst;
    uint32_t ctrla;
    uint32_t ctrlb;
    uint32_t next_ptr;
} hsmci_desc_t;

/* Maximum length of DMA linked list */
#define DMAC_DESC_MAX                   256
#define DMAC_MAX_XFER                   (1024 * 64 - 1)
#define DMAC_XFER_SIZE                  4

typedef struct {
    uintptr_t   base;       /* Virtual base address of HSMCI registers */

    /* DMA specific */
    uintptr_t   dbase;      /* Virtual base address of DMAC registers */

    int         dmachnl;    /* DMA channel within DMAC */
    int         dmarid;     /* DMA request ID */

    sdio_sge_t  sgl[DMAC_DESC_MAX];
    void        *desc;
    paddr_t     pdesc;
} atmel_hc_hsmci_t;

extern int atmel_hsmci_init(sdio_hc_t *hc);
extern int atmel_hsmci_dinit(sdio_hc_t *hc);

#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/devb/sdmmc/sdiodi/hc/atmel-hsmci.h $ $Rev: 813424 $")
#endif
