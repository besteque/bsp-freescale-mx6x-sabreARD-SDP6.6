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

#ifndef	_RCAR_MMCIF_H_INCLUDED
#define	_RCAR_MMCIF_H_INCLUDED

#include <internal.h>
#include <hw/dma.h>

/*
 * MMCIF Memory-mapped registers
 */

#define CONFIG_MMCIF_FREQ       104000000

/* Register list */
#define CE_CMD_SET              0x00000000
#define CE_ARG                  0x00000008
#define CE_ARG_CMD12            0x0000000C
#define CE_CMD_CTRL             0x00000010
#define CE_BLOCK_SET            0x00000014
#define CE_CLK_CTRL             0x00000018
#define CE_BUF_ACC              0x0000001C
#define CE_RESP3                0x00000020
#define CE_RESP2                0x00000024
#define CE_RESP1                0x00000028
#define CE_RESP0                0x0000002C
#define CE_RESP_CMD12           0x00000030
#define CE_DATA                 0x00000034
#define CE_INT                  0x00000040
#define CE_INT_MASK             0x00000044
#define CE_HOST_STS1            0x00000048
#define CE_HOST_STS2            0x0000004C
#define CE_CLK_CTRL2            0x00000070
#define CE_VERSION              0x0000007C

/* CE_CMD_SET */
#define CMD_MASK                0x3f000000
#define CMD_SET_RTYP_NO         ((0 << 23) | (0 << 22))
#define CMD_SET_RTYP_6B         ((0 << 23) | (1 << 22)) // R1/R1b/R3/R4/R5
#define CMD_SET_RTYP_17B        ((1 << 23) | (0 << 22)) // R2
#define CMD_SET_RBSY            (1 << 21)               // R1b
#define CMD_SET_CCSEN           (1 << 20)
#define CMD_SET_WDAT            (1 << 19)               // 1: on data, 0: no data
#define CMD_SET_DWEN            (1 << 18)               // 1: write, 0: read
#define CMD_SET_CMLTE           (1 << 17)               // 1: multi block trans, 0: single
#define CMD_SET_CMD12EN         (1 << 16)               // 1: CMD12 auto issue
#define CMD_SET_RIDXC_INDEX     ((0 << 15) | (0 << 14)) // index check
#define CMD_SET_RIDXC_BITS      ((0 << 15) | (1 << 14)) // check bits check
#define CMD_SET_RIDXC_NO        ((1 << 15) | (0 << 14)) // no check
#define CMD_SET_CRC7C           ((0 << 13) | (0 << 12)) // CRC7 check
#define CMD_SET_CRC7C_BITS      ((0 << 13) | (1 << 12)) // check bits check
#define CMD_SET_CRC7C_INTERNAL  ((1 << 13) | (0 << 12)) // internal CRC7 check
#define CMD_SET_CRC16C          (1 << 10)               // 0: CRC16 check
#define CMD_SET_CRCSTE          (1 << 8)                // 1: not receive CRC status
#define CMD_SET_TBIT            (1 << 7)                // 1: tran mission bit "Low"
#define CMD_SET_OPDM            (1 << 6)                // 1: open/drain
#define CMD_SET_CCSH            (1 << 5)
#define CMD_SET_DATW_1          ((0 << 1) | (0 << 0))   // 1bit
#define CMD_SET_DATW_4          ((0 << 1) | (1 << 0))   // 4bit
#define CMD_SET_DATW_8          ((1 << 1) | (0 << 0))   // 8bit

/* CE_CMD_CTRL */
#define CMD_CTRL_BREAK          (1 << 0)

/* CE_BLOCK_SET */
#define BLOCK_SIZE_MASK         0x0000FFFF

/* CE_CLK_CTRL */
#define CLK_ENABLE              (1 << 24)                                       // 1: output mmc clock
#define CLK_CLEAR               ((1 << 19) | (1 << 18) | (1 << 17) | (1 << 16))
#define CLK_SUP_PCLK            ((1 << 19) | (1 << 18) | (1 << 17) | (1 << 16))
#define SRSPTO_256              ((1 << 13) | (0 << 12))                         // resp timeout
#define SRBSYTO_29              ((1 << 11) | (1 << 10) | (1 << 9)  | (1 << 8))  // resp busy timeout
#define SRWDTO_29               ((1 << 7)  | (1 << 6)  | (1 << 5)  | (1 << 4))  // read/write timeout
#define SCCSTO_29               ((1 << 3)  | (1 << 2)  | (1 << 1)  | (1 << 0))  // ccs timeout

/* CE_BUF_ACC */
#define BUF_ACC_DMAWEN          (1 << 25)
#define BUF_ACC_DMAREN          (1 << 24)
#define BUF_ACC_BUSW_32         (0 << 17)
#define BUF_ACC_BUSW_16         (1 << 17)
#define BUF_ACC_ATYP            (1 << 16)

/* CE_INT */
#define INT_CCSDE               (1 << 29)
#define INT_CMD12DRE            (1 << 26)
#define INT_CMD12RBE            (1 << 25)
#define INT_CMD12CRE            (1 << 24)
#define INT_DTRANE              (1 << 23)
#define INT_BUFRE               (1 << 22)
#define INT_BUFWEN              (1 << 21)
#define INT_BUFREN              (1 << 20)
#define INT_CCSRCV              (1 << 19)
#define INT_RBSYE               (1 << 17)
#define INT_CRSPE               (1 << 16)
#define INT_CMDVIO              (1 << 15)
#define INT_BUFVIO              (1 << 14)
#define INT_WDATERR             (1 << 11)
#define INT_RDATERR             (1 << 10)
#define INT_RIDXERR             (1 << 9)
#define INT_RSPERR              (1 << 8)
#define INT_CCSTO               (1 << 5)
#define INT_CRCSTO              (1 << 4)
#define INT_WDATTO              (1 << 3)
#define INT_RDATTO              (1 << 2)
#define INT_RBSYTO              (1 << 1)
#define INT_RSPTO               (1 << 0)
#define INT_ERR_STS             (INT_CMDVIO  | INT_BUFVIO  | INT_WDATERR |  \
                                 INT_RDATERR | INT_RIDXERR | INT_RSPERR  |  \
                                 INT_CCSTO   | INT_CRCSTO  | INT_WDATTO  |  \
                                 INT_RDATTO  | INT_RBSYTO  | INT_RSPTO)

/* CE_INT_MASK */
#define MASK_ALL                0x00000000
#define MASK_MCCSDE             (1 << 29)
#define MASK_MCMD12DRE          (1 << 26)
#define MASK_MCMD12RBE          (1 << 25)
#define MASK_MCMD12CRE          (1 << 24)
#define MASK_MDTRANE            (1 << 23)
#define MASK_MBUFRE             (1 << 22)
#define MASK_MBUFWEN            (1 << 21)
#define MASK_MBUFREN            (1 << 20)
#define MASK_MCCSRCV            (1 << 19)
#define MASK_MRBSYE             (1 << 17)
#define MASK_MCRSPE             (1 << 16)
#define MASK_MCMDVIO            (1 << 15)
#define MASK_MBUFVIO            (1 << 14)
#define MASK_MWDATERR           (1 << 11)
#define MASK_MRDATERR           (1 << 10)
#define MASK_MRIDXERR           (1 << 9)
#define MASK_MRSPERR            (1 << 8)
#define MASK_MCCSTO             (1 << 5)
#define MASK_MCRCSTO            (1 << 4)
#define MASK_MWDATTO            (1 << 3)
#define MASK_MRDATTO            (1 << 2)
#define MASK_MRBSYTO            (1 << 1)
#define MASK_MRSPTO             (1 << 0)

/* CE_HOST_STS1 */
#define STS1_CMDSEQ             (1 << 31)

/* CE_HOST_STS2 */
#define STS2_CRCSTE             (1 << 31)
#define STS2_CRC16E             (1 << 30)
#define STS2_AC12CRCE           (1 << 29)
#define STS2_RSPCRC7E           (1 << 28)
#define STS2_CRCSTEBE           (1 << 27)
#define STS2_RDATEBE            (1 << 26)
#define STS2_AC12REBE           (1 << 25)
#define STS2_RSPEBE             (1 << 24)
#define STS2_AC12IDXE           (1 << 23)
#define STS2_RSPIDXE            (1 << 22)
#define STS2_CCSTO              (1 << 15)
#define STS2_RDATTO             (1 << 14)
#define STS2_DATBSYTO           (1 << 13)
#define STS2_CRCSTTO            (1 << 12)
#define STS2_AC12BSYTO          (1 << 11)
#define STS2_RSPBSYTO           (1 << 10)
#define STS2_AC12RSPTO          (1 << 9)
#define STS2_RSPTO              (1 << 8)
#define STS2_CRC_ERR            (STS2_CRCSTE    | STS2_CRC16E   | STS2_AC12CRCE  | STS2_RSPCRC7E | STS2_CRCSTEBE)
#define STS2_TIMEOUT_ERR        (STS2_CCSTO     | STS2_RDATTO   |   \
                                 STS2_DATBSYTO  | STS2_CRCSTTO  |   \
                                 STS2_AC12BSYTO | STS2_RSPBSYTO |   \
                                 STS2_AC12RSPTO | STS2_RSPTO)

/* CE_VERSION */
#define SOFT_RST_ON             (1 << 31)
#define SOFT_RST_OFF            (0 << 31)

#define SDHI_TMOUT              1000000

typedef	struct _rcar_mmcif_t {
    void            *bshdl;
    paddr_t         pbase;
    uint32_t        clock;      // MMC clock

    uint32_t        imask;
    uint32_t        datw;
    int             blksz;
    int             busclk;     // MMC bus clock

    uintptr_t       vbase;

    unsigned        trid;       // Transmit DMA request ID
    unsigned        rrid;       // Receive DMA request ID
    int             irq;

    dma_functions_t dmafuncs;
    void            *dma;
#define DMA_DESC_MAX    256
    sdio_sge_t      sgl[DMA_DESC_MAX];
} rcar_mmcif_t;

extern int rcar_mmcif_init(sdio_hc_t *hc);
extern int rcar_mmcif_dinit(sdio_hc_t *hc);

#endif


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/devb/sdmmc/sdiodi/hc/rcar_mmcif.h $ $Rev: 798452 $")
#endif
