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
#include <malloc.h>
#include <unistd.h>
#include <hw/inout.h>
#include <sys/mman.h>

#include <internal.h>

#ifdef SDIO_HC_ATMEL_HSMCI

#include <arm/atsama5d3x.h>
#include <atmel-hsmci.h>

static int atmel_hsmci_pm(sdio_hc_t *hc, int action);
static int atmel_hsmci_cmd(sdio_hc_t *hc, sdio_cmd_t *cmd);
static int atmel_hsmci_abort(sdio_hc_t *hc, sdio_cmd_t *cmd);
static int atmel_hsmci_event(sdio_hc_t *hc, sdio_event_t *ev);
static int atmel_hsmci_cd(sdio_hc_t *hc);
static int atmel_hsmci_pwr(sdio_hc_t *hc, int vdd);
static int atmel_hsmci_clk(sdio_hc_t *hc, int clk);
static int atmel_hsmci_mode(sdio_hc_t *hc, int bus_mode);
static int atmel_hsmci_bus_width(sdio_hc_t *hc, int width);
static int atmel_hsmci_timing(sdio_hc_t *hc, int timing);
static int atmel_hsmci_signal_voltage(sdio_hc_t *hc, int signal_voltage);

static sdio_hc_entry_t atmel_hsmci_hc_entry = {
    .nentries        = 16,
    .dinit           = atmel_hsmci_dinit,
    .pm              = atmel_hsmci_pm,
    .cmd             = atmel_hsmci_cmd,
    .abort           = atmel_hsmci_abort,
    .event           = atmel_hsmci_event,
    .cd              = atmel_hsmci_cd,
    .pwr             = atmel_hsmci_pwr,
    .clk             = atmel_hsmci_clk,
    .bus_mode        = atmel_hsmci_mode,
    .bus_width       = atmel_hsmci_bus_width,
    .timing          = atmel_hsmci_timing,
    .signal_voltage  = atmel_hsmci_signal_voltage,
    .drv_type        = NULL,
    .driver_strength = NULL,
    .tune            = NULL,
    .preset          = NULL,
};


/* Enable HSMCI */
static inline void atmel_hsmci_enable(sdio_hc_t *hc)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    out32(hsmci->base + ATMEL_HSMCI_CR, ATMEL_HSMCI_CR_MCIEN);
}

/* Disable HSMCI */
static inline void atmel_hsmci_disable(sdio_hc_t *hc)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    out32(hsmci->base + ATMEL_HSMCI_CR, ATMEL_HSMCI_CR_MCIDIS);
}

/* Enable HSMCI interrupt */
static inline void atmel_hsmci_interrupt_enable(sdio_hc_t *hc, uint32_t interrupt)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    out32(hsmci->base + ATMEL_HSMCI_IER, interrupt);
}

/* Disable HSMCI interrupt */
static inline void atmel_hsmci_interrupt_disable(sdio_hc_t *hc, uint32_t interrupt)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    out32(hsmci->base + ATMEL_HSMCI_IDR, interrupt);
}

/* Perform software reset of HSMCI and setup defaults */
static void atmel_hsmci_swreset(sdio_hc_t *hc)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    /* Request software reset */
    out32(hsmci->base + ATMEL_HSMCI_CR, ATMEL_HSMCI_CR_SWRST);

    atmel_hsmci_disable(hc);

    /* Configure mode register */
    out32(hsmci->base + ATMEL_HSMCI_MR,
            ATMEL_HSMCI_MR_PWSDIV_MAX | ATMEL_HSMCI_MR_RDPROOF | ATMEL_HSMCI_MR_WRPROOF);

    /* Set maximum data timeout */
    out32(hsmci->base + ATMEL_HSMCI_DTOR, ATMEL_HSMCI_DTOR_MAX);

    /* Enable automatic power saving mode */
    out32(hsmci->base + ATMEL_HSMCI_CR, ATMEL_HSMCI_CR_PWSEN);

    /* Configure DMA controller. */
    if (hc->caps & HC_CAP_DMA) {
        /* Make sure DMAC is enabled */
        if (!(in32(hsmci->dbase + DMAC_EN) & DMAC_EN_ENABLE))
            out32(hsmci->dbase + DMAC_EN, DMAC_EN_ENABLE);

        /* Configure DMA channel */
        uint32_t channel_cfg = 0;
        channel_cfg |= DMAC_CFG_SRC_PER(hsmci->dmarid);
        channel_cfg |= DMAC_CFG_SRC_H2SEL;
        channel_cfg |= DMAC_CFG_DST_PER(hsmci->dmarid);
        channel_cfg |= DMAC_CFG_DST_H2SEL;
        channel_cfg |= DMAC_CFG_AHB_FIFOCFG_HALF;
        channel_cfg |= DMAC_CFG_LOCK_IF;
        channel_cfg |= DMAC_CFG_LOCK_B;
        channel_cfg |= DMAC_CFG_LOCK_IF_L;
        out32(hsmci->dbase + DMAC_CFG(hsmci->dmachnl), channel_cfg);

        /* Configure DMA register */
        out32(hsmci->base + ATMEL_HSMCI_DMA,
                ATMEL_HSMCI_DMA_OFFSET_0 | ATMEL_HSMCI_DMA_CHKSIZE_4 | ATMEL_HSMCI_DMA_DMAEN);
    } else {
        out32(hsmci->base + ATMEL_HSMCI_DMA, 0);
    }

    /* DONE! (HSMCI is disabled) */
}

static int atmel_hsmci_pm(sdio_hc_t *hc, int action)
{
    switch (action) {
        case PM_IDLE:
        case PM_ACTIVE:
            /* Note: we have power saving enabled, so idle is managed by the hardware. */
            /* Start HSMCI peripheral and card clock */
            atmel_hsmci_enable(hc);
            break;

        case PM_SLEEP:
            /* XXX: We might be able to save some more power by turning off the LDO output for the eMMC. */
            /* Stop HSMCI peripheral and card clock */
            atmel_hsmci_disable(hc);
            break;
    }

    return EOK;
}

/* Called from atmel_hsmci_cmd() to setup DMA transfer */
static int atmel_hsmci_dmac_setup(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    atmel_hc_hsmci_t *hsmci  = (atmel_hc_hsmci_t *)hc->cs_hdl;
    sdio_sge_t       *sgp    = cmd->sgl;
    int               sgc    = cmd->sgc;

    if (!(cmd->flags & SCF_DATA_PHYS)) {
        sdio_vtop_sg(cmd->sgl, hsmci->sgl, sgc, cmd->mhdl);
        sgp = hsmci->sgl;
    }

    /* Setup CTRLA */
    uint32_t ctrla = 0;
    ctrla |= DMAC_CTRLA_SRC_WORD;
    ctrla |= DMAC_CTRLA_DST_WORD;
    ctrla |= DMAC_CTRLA_SCSIZE_4;
    ctrla |= DMAC_CTRLA_DCSIZE_4;

    /* Setup CTRLB */
    uint32_t ctrlb = 0;
    ctrlb |= DMAC_CTRLB_IEN;
    if (cmd->flags & SCF_DIR_OUT) {
        ctrlb |= DMAC_CTRLB_SRC_INCR;
        ctrlb |= DMAC_CTRLB_DST_FIXD;
        ctrlb |= DMAC_CTRLB_SIF(ATMEL_AHB_IF1);
        ctrlb |= DMAC_CTRLB_DIF(ATMEL_AHB_IF2);
        ctrlb |= DMAC_CTRLB_FCM2P;
    } else {
        ctrlb |= DMAC_CTRLB_DST_INCR;
        ctrlb |= DMAC_CTRLB_SRC_FIXD;
        ctrlb |= DMAC_CTRLB_SIF(ATMEL_AHB_IF2);
        ctrlb |= DMAC_CTRLB_DIF(ATMEL_AHB_IF1);
        ctrlb |= DMAC_CTRLB_FCP2M;
    }

    /* Disable channel */
    out32(hsmci->dbase + DMAC_CHDR, 1 << hsmci->dmachnl);

    /* Fill descriptors */
    int             sgi, cnt, dlen;
	paddr_t         paddr;
    int             sg_count;
    int             dpnext = (uint32_t)(hsmci->pdesc + sizeof(hsmci_desc_t));
    uint32_t        dport  = (uint32_t)(hc->cfg.base_addr[0] + ATMEL_HSMCIO_FIFO);
    hsmci_desc_t    *desc  = (hsmci_desc_t *)hsmci->desc;
    for (sgi = 0, cnt = 0; sgi < sgc; sgi++, sgp++) {
	    paddr       = sgp->sg_address;
        sg_count    = sgp->sg_count;
        while (sg_count) {
			dlen = min(sg_count, DMAC_MAX_XFER * DMAC_XFER_SIZE);
            if (cmd->flags & SCF_DIR_OUT) {
                desc->src  = paddr;
                desc->dst  = dport;
            } else {
                desc->dst  = paddr;
                desc->src  = dport;
            }
            desc->ctrla    = ctrla | DMAC_CTRLA_BTSIZE(dlen / DMAC_XFER_SIZE);
            desc->ctrlb    = ctrlb;
            desc->next_ptr = dpnext | DMAC_DSCR_AHB_IF(ATMEL_AHB_IF1);

            dpnext        += sizeof(hsmci_desc_t);
            sg_count      -= dlen;
            paddr         += dlen;
			desc++;
            if (++cnt > DMAC_DESC_MAX) {
                return (ENOTSUP);
            }
        }
    }
    (desc - 1)->next_ptr = DMAC_DSCR_AHB_IF(ATMEL_AHB_IF1);

    /* Give first descriptor to DMAC */
    out32(hsmci->dbase + DMAC_DSCR(hsmci->dmachnl), hsmci->pdesc | DMAC_DSCR_AHB_IF(ATMEL_AHB_IF1));

    /* Start DMA transfer */
    out32(hsmci->dbase + DMAC_CHER, 1 << hsmci->dmachnl);

    return (EOK);
}

/* Transfer read data in PIO mode */
static void atmel_hsmci_pio_in_xfer(sdio_hc_t *hc)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    while (in32(hsmci->base + ATMEL_HSMCI_SR) & ATMEL_HSMCI_SR_RXRDY) {
        uint8_t *addr;
        int      len;

        if (sdio_sg_nxt(hc, &addr, &len, 4)) {
            atmel_hsmci_interrupt_disable(hc, ATMEL_HSMCI_SR_RXRDY);
            break;
        }

        *((uint32_t *)addr) = in32(hsmci->base + ATMEL_HSMCI_RDR);
    }
}

/* Transfer write data in PIO mode */
static void atmel_hsmci_pio_out_xfer(sdio_hc_t *hc)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    while (in32(hsmci->base + ATMEL_HSMCI_SR) & ATMEL_HSMCI_SR_TXRDY) {
        uint8_t *addr;
        int      len;

        if (sdio_sg_nxt(hc, &addr, &len, 4)) {
            atmel_hsmci_interrupt_disable(hc, ATMEL_HSMCI_SR_TXRDY);
            break;
        }

        out32(hsmci->base + ATMEL_HSMCI_TDR, *((uint32_t *)addr));
    }
}

static int atmel_hsmci_cmd(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    /* These have to be set in this order to issue the command */
    uint32_t cmdr = ATMEL_HSMCI_CMDR_CMDNB(cmd->opcode);
    uint32_t imr  = ATMEL_HSMCI_SR_XFRDONE;

    /* Set response type */
    if (cmd->flags & SCF_RSP_PRESENT) {
        if (cmd->flags & SCF_RSP_136) {
            cmdr |= ATMEL_HSMCI_CMDR_RSPTYP_136;
        } else if (cmd->flags & SCF_RSP_BUSY) {
            cmdr |= ATMEL_HSMCI_CMDR_RSPTYP_R1B;
        } else {
            cmdr |= ATMEL_HSMCI_CMDR_RSPTYP_48;
        }
    } else {
        cmdr |= ATMEL_HSMCI_CMDR_RSPTYP_NORESP;
    }

    /* Set special command */
    cmdr |= ATMEL_HSMCI_CMDR_SPCMD_STD;

    /* Set open drain command */
    if (hc->bus_mode == BUS_MODE_OPEN_DRAIN) {
        cmdr |= ATMEL_HSMCI_CMDR_OPCMD;
    }

    /* Set max latency */
    cmdr |= ATMEL_HSMCI_CMDR_MAXLAT64;

    /* Set transfer command */
    if (cmd->flags & SCF_DATA_MSK) {
        cmdr |= ATMEL_HSMCI_CMDR_TRCMD_START;
        if (!(hc->caps & HC_CAP_DMA)) {
            if (cmd->flags & SCF_DATA_PHYS ) {
                /* Cannot handle physical addresses in PIO mode */
                return ENOTSUP;
            }
            /* Initialize PIO transfer */
            sdio_sg_start(hc, cmd->sgl, cmd->sgc);

            imr |= (cmd->flags & SCF_DIR_IN) ? ATMEL_HSMCI_SR_RXRDY : ATMEL_HSMCI_SR_TXRDY;
        } else if (atmel_hsmci_dmac_setup(hc, cmd) != EOK) {
            return ENOTSUP;
        }

        /* Set transfer direction */
        cmdr |= (cmd->flags & SCF_DIR_IN) ? ATMEL_HSMCI_CMDR_TDIR_READ : ATMEL_HSMCI_CMDR_TDIR_WRITE;

        /* Set transfer type */
        cmdr |= (cmd->blks > 1) ? ATMEL_HSMCI_CMDR_TRTYP_MULTI : ATMEL_HSMCI_CMDR_TRTYP_SINGLE;
    } else if (cmd->opcode == MMC_STOP_TRANSMISSION) {
        cmdr |= ATMEL_HSMCI_CMDR_TRCMD_STOP;
    } else {
        cmdr |= ATMEL_HSMCI_CMDR_TRCMD_NO_DATA;
    }

    /* Configure block register */
    out32(hsmci->base + ATMEL_HSMCI_BLKR,
        ATMEL_HSMCI_BLKR_BLKLEN(cmd->blksz) | ATMEL_HSMCI_BLKR_BCNT(cmd->blks));

    /* Send command */
    out32(hsmci->base + ATMEL_HSMCI_ARGR, cmd->arg);
    out32(hsmci->base + ATMEL_HSMCI_CMDR, cmdr);
    atmel_hsmci_interrupt_enable (hc,  imr);
    atmel_hsmci_interrupt_disable(hc, ~imr);

    return EOK;
}

static int atmel_hsmci_abort(sdio_hc_t *hc, sdio_cmd_t *cmd)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    /* XXX: There is no good way to abort a command. */

    /* Disable interrupts */
    atmel_hsmci_interrupt_disable(hc, 0xFFFFFFFF);

    /* Wait for transfer done status. */
    while ((in32(hsmci->base + ATMEL_HSMCI_SR) & ATMEL_HSMCI_SR_XFRDONE) == 0) {
        delay(1);
    }

    return EOK;
}

static int atmel_hsmci_event(sdio_hc_t *hc, sdio_event_t *ev)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t*)hc->cs_hdl;
    sdio_cmd_t       *cmd   = hc->wspc.cmd;
    uint32_t         sr, imr, reason;

    /* In case we are called after dinit */
    if (hsmci == NULL) return EOK;

    sr     = in32(hsmci->base + ATMEL_HSMCI_SR);
    imr    = in32(hsmci->base + ATMEL_HSMCI_IMR);
    reason = sr & imr;

    if (reason & ATMEL_HSMCI_SR_RXRDY) {
        atmel_hsmci_pio_in_xfer(hc);
    } else if (reason & ATMEL_HSMCI_SR_TXRDY) {
        atmel_hsmci_pio_out_xfer(hc);
    } else if (reason & ATMEL_HSMCI_SR_XFRDONE) {
        /* Transfer is done.
         * At this point, the command is ready, the data was transmitted, and the card is not busy.
         */

        /* Assume everything went well */
        uint32_t cs = CS_CMD_CMP;

        if ((sr & ATMEL_HSMCI_SR_RTOE) && (cmd->flags & SCF_RSP_PRESENT)) {
            /* Response Time-out Error */
            cs = CS_CMD_TO_ERR;
        } else if ((sr & ATMEL_HSMCI_SR_RCRCE) && (cmd->flags & SCF_RSP_CRC)) {
            /* Response CRC Error */
            cs = CS_CMD_CRC_ERR;
        } else if (sr & ATMEL_HSMCI_SR_RINDE) {
            /* Response Index Error */
            cs = CS_CMD_IDX_ERR;
        } else if (sr & ATMEL_HSMCI_SR_RENDE) {
            /* Response End Bit Error */
            cs = CS_CMD_END_ERR;
        } else if (sr & ATMEL_HSMCI_SR_DCRCE) {
            /* Data CRC Error */
            cs = CS_DATA_CRC_ERR;
        } else if (sr & ATMEL_HSMCI_SR_DTOE) {
            /* Data Time-out Error */
            cs = CS_DATA_TO_ERR;
        } else if (sr & ATMEL_HSMCI_SR_CSTOE) {
            /* Completion Signal Time-out Error */
            cs = CS_CMD_CMP_ERR;
        } else if (sr & ATMEL_HSMCI_SR_OERRORS) {
            /* XXX: Other errors that don't have SDMMC equivalents:
             *   - Response Direction Error
             *   - DMA Block Overrun Error
             *   - ACKRCVE: Boot Operation Acknowledge Error
             */
            cs = CS_CMD_END_ERR;
        } else if (cmd->flags & SCF_RSP_136) {
            cmd->rsp[0] = in32(hsmci->base + ATMEL_HSMCI_RSPR);
            cmd->rsp[1] = in32(hsmci->base + ATMEL_HSMCI_RSPR);
            cmd->rsp[2] = in32(hsmci->base + ATMEL_HSMCI_RSPR);
            cmd->rsp[3] = in32(hsmci->base + ATMEL_HSMCI_RSPR);
        } else if (cmd->flags & SCF_RSP_PRESENT) {
            cmd->rsp[0] = in32(hsmci->base + ATMEL_HSMCI_RSPR);
        }

        sdio_cmd_cmplt(hc, cmd, cs);
        atmel_hsmci_interrupt_disable(hc, imr);
    }

    InterruptUnmask(hc->cfg.irq[0], hc->hc_iid);

    return EOK;
}

static int atmel_hsmci_cd(sdio_hc_t *hc)
{
    /* XXX: No real card detect for now.
     * The driver will be mainly used with eMMC, so card detect is not necessary.
     */
    return SDIO_TRUE;
}

static int atmel_hsmci_pwr(sdio_hc_t *hc, int vdd)
{
    /* XXX: We don't actually set voltages.
     * This would have to be board specific and the eMMC is always at 1.8V anyway.
     */
    hc->vdd = vdd;
    return EOK;
}

static int atmel_hsmci_clk(sdio_hc_t *hc, int clk)
{
    sdio_hc_cfg_t       *cfg   = &hc->cfg;
    atmel_hc_hsmci_t    *hsmci = (atmel_hc_hsmci_t*)hc->cs_hdl;
    uint32_t            divider, clkdiv, clkodd, mr;

    /* High Speed MultiMedia Card Interface clock (MCCK or HSMCI_CK) is Master Clock (MCK) divided by ({CLKDIV,CLKODD}+2). */

    clk = min(clk, cfg->clk / 2);
    clk = max(clk, cfg->clk / (511 + 2));

    /* Calculate divider */
    divider = (cfg->clk + clk - 1 ) / clk;

    /* Calculate CLKDIV and CLKODD values. */
    clkdiv = (divider > 2) ? ((divider - 2) / 2) : 0;
    clkodd = (divider > 2) ? ((divider - 2) % 2) : 0;

    /* Configure MR */
    mr = in32(hsmci->base + ATMEL_HSMCI_MR);
    mr &= ~(ATMEL_HSMCI_MR_CLKODD | ATMEL_HSMCI_MR_CLKDIV_MASK);
    mr |= ATMEL_HSMCI_MR_CLKDIV(clkdiv);
    mr |= clkodd ? ATMEL_HSMCI_MR_CLKODD : 0;
    out32(hsmci->base + ATMEL_HSMCI_MR, mr);

    /* Calculate real clock */
    hc->clk = cfg->clk / divider;

    return EOK;
}

static int atmel_hsmci_mode(sdio_hc_t *hc, int bus_mode)
{
    hc->bus_mode = bus_mode;
    return EOK;
}

static int atmel_hsmci_bus_width(sdio_hc_t *hc, int width)
{
    atmel_hc_hsmci_t    *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;
    uint32_t            sdcr = ATMEL_HSMCI_SDCR_SDCSEL_SLOTA;

    switch (width) {
        default:
        case BUS_WIDTH_1:
            sdcr |= ATMEL_HSMCI_SDCR_SDCBUS_BW1;
            break;
        case BUS_WIDTH_4:
            sdcr |= ATMEL_HSMCI_SDCR_SDCBUS_BW4;
            break;
        case BUS_WIDTH_8:
            sdcr |= ATMEL_HSMCI_SDCR_SDCBUS_BW8;
            break;
    }

    out32(hsmci->base + ATMEL_HSMCI_SDCR, sdcr);

    hc->bus_width = width;

    return EOK;
}

static int atmel_hsmci_timing(sdio_hc_t *hc, int timing)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    switch(timing) {
        case TIMING_HS:
            out32(hsmci->base + ATMEL_HSMCI_CFG, ATMEL_HSMCI_CFG_HSMODE);
            hc->timing = TIMING_HS;
            break;

        default:
        case TIMING_LS:
            out32(hsmci->base + ATMEL_HSMCI_CFG, 0);
            hc->timing = TIMING_LS;
            break;
    }

    return EOK;
}

static int atmel_hsmci_signal_voltage(sdio_hc_t *hc, int signal_voltage)
{
    /* XXX: We don't actually set voltages.
     * This would have to be board specific and the eMMC is always at 1.8V anyway.
     */
    hc->signal_voltage = signal_voltage;
    return EOK;
}

int atmel_hsmci_dinit(sdio_hc_t *hc)
{
    atmel_hc_hsmci_t *hsmci = (atmel_hc_hsmci_t *)hc->cs_hdl;

    /* Disable peripheral */
    atmel_hsmci_disable(hc);

    /* Detach Interrupt */
    InterruptDetach(hc->hc_iid);

    /* Unmap peripheral */
    munmap_device_io(hsmci->base, hc->cfg.base_addr_size[0]);

    if (hc->caps & HC_CAP_DMA) {
        /* Unmap DMAC, DMAC descriptors */
        munmap_device_io(hsmci->dbase, SAMA5D3X_DMAC_SIZE);
        sdio_free(hsmci->desc, sizeof(hsmci_desc_t) * DMAC_DESC_MAX);
    }

    /* Free allocated memory */
    free(hsmci);
    hc->cs_hdl = NULL;

    return EOK;
}

int atmel_hsmci_init(sdio_hc_t *hc)
{
    sdio_hc_cfg_t       *cfg = &hc->cfg;
    atmel_hc_hsmci_t    *hsmci;
    struct sigevent     event;

    memcpy(&hc->entry, &atmel_hsmci_hc_entry, sizeof(sdio_hc_entry_t));

    if (!cfg->base_addr[0])
        return (ENODEV);

    if (!cfg->base_addr_size[0])
        cfg->base_addr_size[0] = ATMEL_HSMCI_SIZE;

    if ((hsmci = hc->cs_hdl = calloc(1, sizeof(*hsmci))) == NULL)
        return ENOMEM;

    if (cfg->clk == 0)
        cfg->clk = ATMEL_HSMCI_MCK;

    /* For SAMA5D3x */
    hc->clk_max = cfg->clk / 2;
    hc->clk_min	= cfg->clk / (511 + 2);

    /* Set caps */
    hc->ocr     = OCR_VDD_32_33 | OCR_VDD_30_31 | OCR_VDD_29_30 | OCR_VDD_17_195;
    hc->caps   |= HC_CAP_BW4 | HC_CAP_BW8;
    hc->caps   |= HC_CAP_PIO;
    hc->caps   |= HC_CAP_HS;
    hc->caps   |= HC_CAP_SV_3_3V | HC_CAP_SV_1_8V;
    hc->caps   |= HC_CAP_SLEEP;

    /* Map HSMCI registers */
    hsmci->base = mmap_device_io(cfg->base_addr_size[0], cfg->base_addr[0]);
    if (hsmci->base == MAP_DEVICE_FAILED) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s: mmap_device_io(0x%llx) %s", __FUNCTION__, cfg->base_addr[0], strerror(errno));
        free(hsmci);
        return (ENODEV);;
    }

    /* Map DMAC base address is specified */
    if (cfg->base_addr[1] != 0) {
        /* Map DMAC registers */
        hsmci->dbase = mmap_device_io(SAMA5D3X_DMAC_SIZE, cfg->base_addr[1]);
        if (hsmci->dbase == MAP_DEVICE_FAILED) {
            sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s: mmap_device_io(0x%llx) %s", __FUNCTION__, cfg->base_addr[1], strerror(errno));
            munmap_device_io(hsmci->base, cfg->base_addr_size[0]);
            free(hsmci);
            return (ENODEV);
        }

        /* Map DMA descriptors */
        if ((hsmci->desc = sdio_alloc(sizeof(hsmci_desc_t) * DMAC_DESC_MAX)) == NULL) {
            sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s: descriptor mmap %s", __FUNCTION__, strerror(errno));
            munmap_device_io(hsmci->dbase, SAMA5D3X_DMAC_SIZE);
            munmap_device_io(hsmci->base,  cfg->base_addr_size[0]);
            free(hsmci);
            return (ENODEV);
        }

        hc->caps |= HC_CAP_DMA;
        hc->cfg.sg_max = DMAC_DESC_MAX;

        hsmci->pdesc = sdio_vtop(hsmci->desc);

        hsmci->dmachnl = hc->cfg.dma_chnl[0];   // The first DMA option is DMA channel
        hsmci->dmarid  = hc->cfg.dma_chnl[1];   // The second DMA option is DMA request ID
    }

    /* Software reset */
    atmel_hsmci_swreset(hc);

    /* reconcile command line options */
    hc->caps &= cfg->caps;

    /* Attach interrupt */
    SIGEV_PULSE_INIT(&event, hc->hc_coid, SDIO_PRIORITY, HC_EV_INTR, NULL);
    if ((hc->hc_iid = InterruptAttachEvent(cfg->irq[0], &event, _NTO_INTR_FLAGS_TRK_MSK)) == -1) {
        sdio_slogf(_SLOGC_SDIODI, _SLOG_ERROR, 1, 1, "%s: InterruptAttachEvent(%d) %s", __FUNCTION__, cfg->irq[0], strerror(errno));
        if (hc->caps & HC_CAP_DMA) {
            sdio_free(hsmci->desc, sizeof(hsmci_desc_t) * DMAC_DESC_MAX);
            munmap_device_io(hsmci->dbase, SAMA5D3X_DMAC_SIZE);
        }
        munmap_device_io(hsmci->base, cfg->base_addr_size[0]);
        free(hsmci);
        return (ENODEV);
    }

    /* Enable */
    atmel_hsmci_enable(hc);

    return EOK;
}

#endif


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/devb/sdmmc/sdiodi/hc/atmel-hsmci.c $ $Rev: 813424 $")
#endif
