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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <hw/inout.h>
#include <sys/mman.h>
#include <internal.h>
#include <sys/syspage.h>
#include <inttypes.h>

#ifdef SDIO_HC_RCAR_MMCIF
#include <rcar_mmcif.h>
#include <arm/r-car.h>

static int rcar_mmcif_reset(sdio_hc_t *hc);

static inline uint32_t mmcif_read(uintptr_t base, int reg)
{
    return in32(base + reg);
}

static inline void mmcif_write(uintptr_t base, int reg, uint32_t val)
{
    out32(base + reg, val);
}

static inline void mmcif_bitset(uintptr_t base, int reg, uint32_t val)
{
    out32(base + reg, val | in32(base + reg));
}

static inline void mmcif_bitclr(uintptr_t base, int reg, uint32_t val)
{
    out32(base + reg, ~val & in32(base + reg));
}


static int rcar_mmcif_intr_event(sdio_hc_t *hc)
{
    rcar_mmcif_t    *mmcif;
    sdio_cmd_t      *cmd;
    uint32_t        stat, rstat;
    int             cs = CS_CMD_INPROG;

    mmcif  = (rcar_mmcif_t *)hc->cs_hdl;

    rstat = mmcif_read(mmcif->vbase, CE_INT);
    stat  = rstat & mmcif_read(mmcif->vbase, CE_INT_MASK);


    mmcif_bitclr(mmcif->vbase, CE_INT, stat);

    if ((cmd = hc->wspc.cmd) == NULL)
        return (EOK);

    /* Check of errors */
    if (stat & INT_ERR_STS) {
        uint32_t    hsts1, hsts2;

        hsts1 = mmcif_read(mmcif->vbase, CE_HOST_STS1);
        hsts2 = mmcif_read(mmcif->vbase, CE_HOST_STS2);

        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1,
                        "%s, ERROR in HC, CMD%d, rsts = %x, status %x : %x", __FUNCTION__, cmd->opcode, rstat, hsts1, hsts2);

        if (hsts2 & STS2_CRC16E)            cs = CS_DATA_CRC_ERR;
        if (hsts2 & (STS2_AC12CRCE | STS2_RSPCRC7E))    cs = CS_CMD_CRC_ERR;
        if (hsts2 & STS2_RDATTO)            cs = CS_DATA_TO_ERR;
        if (hsts2 & (STS2_RSPIDXE | STS2_AC12IDXE))     cs = CS_CMD_IDX_ERR;
        if (hsts2 & STS2_RDATEBE)           cs = CS_DATA_END_ERR;
        if (hsts2 & STS2_RSPEBE)            cs = CS_CMD_END_ERR;
        if (hsts2 & STS2_RSPTO)             cs = CS_CMD_TO_ERR;
        if (!cs)                            cs = CS_CMD_CMP_ERR;

        if (hsts1 & STS1_CMDSEQ) {
            int timeout;

            mmcif_bitset(mmcif->vbase, CE_CMD_CTRL, CMD_CTRL_BREAK);
            mmcif_bitclr(mmcif->vbase, CE_CMD_CTRL, CMD_CTRL_BREAK);
            timeout = 10000;
            while (timeout--) {
                if (!(mmcif_read(mmcif->vbase, CE_HOST_STS1) & STS1_CMDSEQ))
                    break;
                nanospin_ns(1000);
            }
            rcar_mmcif_reset(hc);
        }
	} else {
        if (stat & (INT_CMD12DRE | INT_CMD12RBE))
            mmcif_write(mmcif->vbase, CE_INT, ~(INT_CMD12DRE | INT_CMD12RBE | INT_CMD12CRE | INT_BUFRE));

        if (rstat & (INT_CRSPE | INT_RBSYE)) {
            if ((cmd->flags & SCF_RSP_136)) {
                cmd->rsp[0] = mmcif_read(mmcif->vbase, CE_RESP3);
                cmd->rsp[1] = mmcif_read(mmcif->vbase, CE_RESP2);
                cmd->rsp[2] = mmcif_read(mmcif->vbase, CE_RESP1);
                cmd->rsp[3] = mmcif_read(mmcif->vbase, CE_RESP0);
            } else {
                cmd->rsp[0] = mmcif_read(mmcif->vbase, CE_RESP0);
            }
        }

        /* End of command */
        if (stat & (INT_CRSPE | INT_RBSYE)) {
            cs = CS_CMD_CMP;
        }
        if (stat & (INT_BUFRE | INT_DTRANE)) {
            /* End of data transfer */
            cs = CS_CMD_CMP;
            mmcif->dmafuncs.xfer_complete(mmcif->dma);
        } 
    }


    if (cs != CS_CMD_INPROG) {
        if (cmd->flags & SCF_CTYPE_ADTC) {
            mmcif_bitclr(mmcif->vbase, CE_BUF_ACC, cmd->flags & SCF_DIR_IN ? BUF_ACC_DMAREN : BUF_ACC_DMAWEN);
            mmcif->dmafuncs.xfer_complete(mmcif->dma);
        }
        sdio_cmd_cmplt(hc, cmd, cs);
    }

    return (EOK);
}

static int rcar_mmcif_event(sdio_hc_t *hc, sdio_event_t *ev)
{
    rcar_mmcif_t    *mmcif = (rcar_mmcif_t *)hc->cs_hdl;
    int             status = CS_CMD_INPROG;

    switch (ev->code) {
        case HC_EV_INTR:
            status = rcar_mmcif_intr_event(hc);
            InterruptUnmask(mmcif->irq, hc->hc_iid);
            break;

        default:
            break;
    }

    return (status);
}

static int rcar_mmcif_dma_setup(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    rcar_mmcif_t    *mmcif;
    dma_transfer_t  tinfo;
    dma_addr_t      saddr, daddr;
    dma_addr_t      *sgaddr;
    sdio_sge_t      *sgp;
    int             sgc, sgi;

    mmcif = (rcar_mmcif_t *)hc->cs_hdl;

    sgc = cmd->sgc;
    sgp = cmd->sgl;

    if (!(cmd->flags & SCF_DATA_PHYS)) {
        sdio_vtop_sg(sgp, mmcif->sgl, sgc, cmd->mhdl);
        sgp = mmcif->sgl;
    }

    memset(&tinfo, 0, sizeof(tinfo));

    tinfo.xfer_unit_size = 4;
    tinfo.xfer_bytes     = cmd->blksz * cmd->blks;

    if (cmd->flags & SCF_DIR_IN) {
        mmcif_bitset(mmcif->vbase, CE_BUF_ACC, BUF_ACC_DMAREN);

        saddr.paddr      = mmcif->pbase + CE_DATA;
        tinfo.src_addrs  = &saddr;

        if (sgc > 1) {
            sgaddr = alloca(sgc * sizeof(dma_addr_t));

            tinfo.dst_flags    |= DMA_ADDR_FLAG_SEGMENTED;
            tinfo.dst_fragments = sgc;
            tinfo.dst_addrs     = sgaddr;
            for (sgi = 0; sgi < sgc; sgi++, sgaddr++, sgp++) {
                sgaddr->paddr   = sgp->sg_address;
                sgaddr->len     = sgp->sg_count;
            }
        } else {
            daddr.paddr         = sgp->sg_address;
            tinfo.dst_addrs     = &daddr;
        }
        tinfo.src_flags  = DMA_ADDR_FLAG_NO_INCREMENT | DMA_ADDR_FLAG_DEVICE;
        tinfo.req_id     = mmcif->rrid;     // Receive DMA request ID
    } else {
        mmcif_bitset(mmcif->vbase, CE_BUF_ACC, BUF_ACC_DMAWEN);

        if (sgc > 1) {
            sgaddr = alloca(sgc * sizeof(dma_addr_t));

            tinfo.src_flags    |= DMA_ADDR_FLAG_SEGMENTED;
            tinfo.src_fragments = sgc;
            tinfo.src_addrs     = sgaddr;
            for (sgi = 0; sgi < sgc; sgi++, sgaddr++, sgp++) {
                sgaddr->paddr   = sgp->sg_address;
                sgaddr->len     = sgp->sg_count;
            }
        } else {
            saddr.paddr         = sgp->sg_address;
            tinfo.src_addrs     = &saddr;
        }

        daddr.paddr      = mmcif->pbase + CE_DATA;
        tinfo.dst_addrs  = &daddr;
        tinfo.dst_flags  = DMA_ADDR_FLAG_NO_INCREMENT | DMA_ADDR_FLAG_DEVICE;
        tinfo.req_id     = mmcif->trid;     // Transmit DMA request ID
    }

    mmcif->dmafuncs.setup_xfer(mmcif->dma, &tinfo);
    mmcif->dmafuncs.xfer_start(mmcif->dma);

    return (EOK);
}

static int rcar_mmcif_xfer_setup(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    rcar_mmcif_t    *mmcif;
    int             status = EOK;

    mmcif = (rcar_mmcif_t *)hc->cs_hdl;

    sdio_sg_start(hc, cmd->sgl, cmd->sgc);

    if (cmd->sgc && (hc->caps & HC_CAP_DMA)) {
        if ((status = rcar_mmcif_dma_setup(hc, cmd)) == EOK) {
        }
    }

    // block size
    mmcif_write(mmcif->vbase, CE_BLOCK_SET,
            (mmcif_read(mmcif->vbase, CE_BLOCK_SET) & 0xFFFF0000) | cmd->blksz);

    /* only valid for multi-block transfer */
    if (cmd->blks > 1)
        mmcif_write(mmcif->vbase, CE_BLOCK_SET,
                (mmcif_read(mmcif->vbase, CE_BLOCK_SET) & 0x0000FFFF) | (cmd->blks << 16));

    return (status);
}

static uint32_t mmcif_set_cmd(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    rcar_mmcif_t    *mmcif;
    uint32_t        opc;
    uint32_t        tmp;

    mmcif = (rcar_mmcif_t *)hc->cs_hdl;
    opc   = cmd->opcode;
    tmp   = mmcif->datw;

    /* Response Type check */
    if (cmd->flags & SCF_RSP_PRESENT) {
        if (cmd->flags & SCF_RSP_136)	// 136 bit response
            tmp |= CMD_SET_RTYP_17B | CMD_SET_CRC7C_INTERNAL | CMD_SET_RIDXC_BITS;
        else
            tmp |= CMD_SET_RTYP_6B;

        if (!(cmd->flags & SCF_RSP_CRC))
            tmp |= CMD_SET_CRC7C_BITS | CMD_SET_RIDXC_BITS;

        if (cmd->flags & SCF_RSP_BUSY)
            tmp |= CMD_SET_RBSY;
    }

    if (cmd->flags & SCF_CTYPE_ADTC) {
        tmp |= CMD_SET_WDAT;

        if (cmd->flags & SCF_DIR_IN) {
            mmcif->imask |= INT_BUFRE;
        } else {
            mmcif->imask |= INT_DTRANE;
            tmp |= CMD_SET_DWEN;
        }

        if (cmd->flags & SCF_MULTIBLK) {
            tmp |= CMD_SET_CMLTE;
            if (hc->caps & HC_CAP_ACMD12)
                tmp |= CMD_SET_CMD12EN;
        }
    } else if (cmd->flags & SCF_RSP_BUSY)
        mmcif->imask |= INT_RBSYE;
    else
        mmcif->imask |= INT_CRSPE;

    return ((opc << 24) | tmp);
}

static int rcar_mmcif_cmd(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    rcar_mmcif_t    *mmcif;
    int             status, i;
    uint32_t        opc;

    mmcif = (rcar_mmcif_t *)hc->cs_hdl;

    if (mmcif_read(mmcif->vbase, CE_HOST_STS1) & STS1_CMDSEQ) {
        for (i = 100000; i >= 0; i--) {
            if (!(mmcif_read(mmcif->vbase, CE_HOST_STS1) & STS1_CMDSEQ))
                break;
            nanospin_ns(1000);
        }

        if (i <= 0)
            return (EAGAIN);
    }

    mmcif->imask = MASK_MCMDVIO | MASK_MBUFVIO | MASK_MWDATERR | MASK_MRDATERR
                        | MASK_MRIDXERR | MASK_MRSPERR | MASK_MCCSTO | MASK_MCRCSTO
                        | MASK_MWDATTO | MASK_MRDATTO | MASK_MRBSYTO | MASK_MRSPTO;

    opc = mmcif_set_cmd(hc, cmd);

    if (cmd->flags & SCF_CTYPE_ADTC) {
        if ((status = rcar_mmcif_xfer_setup(hc, cmd)) != EOK)
            return (status);
    }

    mmcif_write(mmcif->vbase, CE_INT, 0xD80430C0);
    mmcif_write(mmcif->vbase, CE_INT_MASK, mmcif->imask);

    /* set arg */
    mmcif_write(mmcif->vbase, CE_ARG, cmd->arg);

    /* set cmd */
    mmcif_write(mmcif->vbase, CE_CMD_SET, opc);

    return (EOK);
}

static int rcar_mmcif_abort(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    return (EOK);
}

static int rcar_mmcif_pwr(sdio_hc_t *hc, int vdd)
{
    hc->vdd = vdd;

    return (EOK);
}

static int rcar_mmcif_bus_mode(sdio_hc_t *hc, int bus_mode)
{
    hc->bus_mode = bus_mode;

    return (EOK);
}

static int rcar_mmcif_bus_width(sdio_hc_t *hc, int width)
{
    rcar_mmcif_t *mmcif;

    mmcif = (rcar_mmcif_t *)hc->cs_hdl;

    switch (width) {
        case 8:
            mmcif->datw = CMD_SET_DATW_8;
            break;
        case 4:
            mmcif->datw = CMD_SET_DATW_4;
            break;
        case 1:
            mmcif->datw = CMD_SET_DATW_1;
            break;
        default:
            return (EINVAL);
    }

    hc->bus_width = width;

    return (EOK);
}

static int rcar_mmcif_clk(sdio_hc_t *hc, int clk)
{
    rcar_mmcif_t    *mmcif;
    uint32_t        clkdiv;
    uint32_t        clock;

    mmcif = (rcar_mmcif_t *)hc->cs_hdl;

    clock = mmcif->clock >> 1;

    for (clkdiv = 0, clock <<= 1; clkdiv <= 9; clkdiv++) {
        if ((clock >>= 1) <= clk)
            break;
    }

    hc->clk = mmcif->busclk = clock;

    mmcif_bitclr(mmcif->vbase, CE_CLK_CTRL, CLK_ENABLE | (0x0F << 16));
    mmcif_bitset(mmcif->vbase, CE_CLK_CTRL, clkdiv << 16);
    mmcif_bitset(mmcif->vbase, CE_CLK_CTRL, CLK_ENABLE);

    return (EOK);
}

static int rcar_mmcif_signal_voltage(sdio_hc_t *hc, int signal_voltage)
{
    return (EOK);
}

static int rcar_mmcif_timing(sdio_hc_t *hc, int timing)
{
    hc->timing = timing;

    //rcar_mmcif_clk(hc, hc->clk);

    return (EOK);
}

static int rcar_mmcif_cd(sdio_hc_t *hc)
{
    return (CD_INS);
}

int rcar_mmcif_dinit(sdio_hc_t *hc)
{
    rcar_mmcif_t *mmcif;

    if (!hc || !hc->cs_hdl)
        return (EOK);

    mmcif = (rcar_mmcif_t *)hc->cs_hdl;

    if (mmcif->vbase) {
        if (hc->hc_iid != -1)
            InterruptDetach(hc->hc_iid);

        munmap_device_io(mmcif->vbase, RCAR_MMCIF_SIZE);
    }

    if (mmcif->dma) {
        mmcif->dmafuncs.channel_release(mmcif->dma);
        mmcif->dmafuncs.fini();
    }

    free(mmcif);
    hc->cs_hdl = NULL;

    return (EOK);
}

static int rcar_mmcif_reset(sdio_hc_t *hc)
{
    rcar_mmcif_t    *mmcif = (rcar_mmcif_t *)hc->cs_hdl;

    mmcif_write(mmcif->vbase, CE_VERSION, SOFT_RST_ON);
    delay(1);
    mmcif_write(mmcif->vbase, CE_VERSION, SOFT_RST_OFF);
    delay(1);

    mmcif_write(mmcif->vbase, CE_INT_MASK, MASK_ALL);
    mmcif_write(mmcif->vbase, CE_INT, 0xD80430C0);

    /* byte swap on */
    mmcif_write(mmcif->vbase, CE_BUF_ACC, BUF_ACC_ATYP);

    mmcif_write(mmcif->vbase, CE_CLK_CTRL, SRSPTO_256 | SRBSYTO_29 | SRWDTO_29 | SCCSTO_29);

    // configure clock
    rcar_mmcif_clk(hc, mmcif->busclk);

    return (EOK);
}

static sdio_hc_entry_t rcar_mmcif_entry = {
    16,
    rcar_mmcif_dinit, NULL,
    rcar_mmcif_cmd, rcar_mmcif_abort,
    rcar_mmcif_event, rcar_mmcif_cd, rcar_mmcif_pwr,
    rcar_mmcif_clk, rcar_mmcif_bus_mode,
    rcar_mmcif_bus_width, rcar_mmcif_timing,
    rcar_mmcif_signal_voltage, NULL, NULL, NULL, NULL
};

static int rcar_mmcif_init_dma(sdio_hc_t *hc)
{
    sdio_hc_cfg_t       *cfg = &hc->cfg;
    rcar_mmcif_t        *mmcif = (rcar_mmcif_t *)hc->cs_hdl;
    dma_channel_query_t query;
    dma_channel_info_t  info;

    if (get_dmafuncs(&mmcif->dmafuncs, sizeof(mmcif->dmafuncs)) == -1) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: Failed to get DMA lib functions", __FUNCTION__);
        return ENODEV;
    }

    mmcif->dmafuncs.init(NULL);

    if ((mmcif->dma = mmcif->dmafuncs.channel_attach(NULL, NULL, NULL, 0, DMA_ATTACH_ANY_CHANNEL)) == NULL) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: Unable to attach to Rx DMA Channel", __FUNCTION__);
        return ENODEV;
    }

    mmcif->dmafuncs.query_channel(mmcif->dma, &query);
    mmcif->dmafuncs.channel_info(query.chan_idx, &info);

    cfg->sg_max = info.max_src_segments > 1 ?  info.max_src_segments : 1;

    return (EOK);
}

int rcar_mmcif_init(sdio_hc_t *hc)
{
    sdio_hc_cfg_t   *cfg;
    rcar_mmcif_t    *mmcif;
    struct sigevent event;

    hc->hc_iid  = -1;
    cfg         = &hc->cfg;

    memcpy(&hc->entry, &rcar_mmcif_entry, sizeof(sdio_hc_entry_t));

    if ((mmcif = hc->cs_hdl = calloc(1, sizeof(rcar_mmcif_t))) == NULL)
        return (ENOMEM);

    if (cfg->base_addrs > 0 && cfg->dma_chnls > 0 && cfg->irqs > 0) {
        mmcif->pbase = cfg->base_addr[0];
        mmcif->trid  = cfg->dma_chnl[0];
        mmcif->rrid  = cfg->dma_chnls > 0 ? cfg->dma_chnl[1] : mmcif->trid + 1;
        mmcif->irq   = cfg->irq[0];
    } else if (cfg->idx == 0) {
        mmcif->pbase = RCAR_MMCIF0_BASE;
        mmcif->trid  = RCAR_DREQ_MMCIF0_TXI;
        mmcif->rrid  = RCAR_DREQ_MMCIF0_RXI;
        mmcif->irq   = RCAR_INTCYSY_MMC0;
    } else if (cfg->idx == 1) {
        mmcif->pbase = RCAR_MMCIF1_BASE;
        mmcif->trid  = RCAR_DREQ_MMCIF1_TXI;
        mmcif->rrid  = RCAR_DREQ_MMCIF1_RXI;
        mmcif->irq   = RCAR_INTCYSY_MMC1;
    } else {
        rcar_mmcif_dinit(hc);
        return (ENODEV);
    }

    if ((mmcif->vbase = (uintptr_t)mmap_device_io(RCAR_MMCIF_SIZE, mmcif->pbase)) == (uintptr_t)MAP_FAILED) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1,
            "%s: MMCIF base mmap_device_io (0x%x) %s", __FUNCTION__, mmcif->pbase, strerror(errno));
        rcar_mmcif_dinit(hc);
        return (errno);
    }

    if (cfg->clk)
        mmcif->clock = cfg->clk;
    else
        mmcif->clock = 104 * 1000 * 1000;;

    hc->clk_max = mmcif->clock;

    hc->caps |= HC_CAP_BSY | HC_CAP_BW4 | HC_CAP_BW8;
    hc->caps |= HC_CAP_ACMD12;
    hc->caps |= HC_CAP_HS;
    hc->caps |= HC_CAP_DMA;
    hc->caps |= HC_CAP_SV_3_3V;

    hc->ocr   = OCR_VDD_32_33 | OCR_VDD_33_34;

    hc->caps &= cfg->caps;      /* reconcile command line options */

    if (rcar_mmcif_init_dma(hc) != EOK) {
        rcar_mmcif_dinit(hc);
        return (ENODEV);
    }

    mmcif->busclk = 400 * 1000;      // 400KHz clock for ident
    rcar_mmcif_reset(hc);

    SIGEV_PULSE_INIT(&event, hc->hc_coid, SDIO_PRIORITY, HC_EV_INTR, NULL);
    if ((hc->hc_iid = InterruptAttachEvent(mmcif->irq, &event, _NTO_INTR_FLAGS_TRK_MSK)) == -1) {
        rcar_mmcif_dinit(hc);
        return (errno);
    }

    return (EOK);
}

#endif


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/devb/sdmmc/sdiodi/hc/rcar_mmcif.c $ $Rev: 798452 $")
#endif
