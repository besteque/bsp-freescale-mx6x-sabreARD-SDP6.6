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

#ifdef SDIO_HC_PXIV

//#define PXIV_SDMMC_DEBUG
#define PXIV_DMA_SUPPORTED

#include <pxiv.h>

static int pxiv_pwr( sdio_hc_t *hc, int vdd );
static int pxiv_bus_mode( sdio_hc_t *hc, int bus_mode );
static int pxiv_bus_width( sdio_hc_t *hc, int width );
static int pxiv_clk( sdio_hc_t *hc, int clk );
static int pxiv_timing( sdio_hc_t *hc, int timing );
static int pxiv_signal_voltage( sdio_hc_t *hc, int signal_voltage );
static int pxiv_cd( sdio_hc_t *hc );
static int pxiv_cmd( sdio_hc_t *hc, sdio_cmd_t *cmd );
static int pxiv_abort( sdio_hc_t *hc, sdio_cmd_t *cmd );
static int pxiv_event( sdio_hc_t *hc, sdio_event_t *ev );

static sdio_hc_entry_t pxiv_sdmmc_hc_entry = {
    16,
    pxiv_dinit, NULL,
    pxiv_cmd, pxiv_abort,
    pxiv_event, pxiv_cd, pxiv_pwr,
    pxiv_clk, pxiv_bus_mode,
    pxiv_bus_width, pxiv_timing,
    pxiv_signal_voltage, NULL,
    NULL, NULL, NULL
};

static inline void out32_mask( uint32_t addr, uint32_t mask, uint32_t data )
{
    out32( addr, (in32(addr) & ~mask) | (data & mask) );
}

static int pxiv_waitmask( sdio_hc_t *hc, uint32_t reg, uint32_t mask, uint32_t val, uint32_t usec )
{
    pxiv_sdmmc_hc_t *sdhc;
    uint32_t        cnt;
    int             stat;
    int             rval;
    uint32_t        iter;

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    stat = ETIMEDOUT;
    rval = 0;

    /* fast poll for 1ms - 1us intervals */
    for( cnt = min( usec, 1000 ); cnt; cnt-- ) {
        if( ( ( rval = in32( sdhc->base + reg ) ) & mask ) == val ) {
            stat = EOK;
            break;
        }
        nanospin_ns( 1000L );
    }

    if( usec > 1000 && stat ) {
        iter = usec / 1000L;
        for( cnt = iter; cnt; cnt-- ) {
            if( ( ( rval = in32( sdhc->base + reg ) ) & mask ) == val ) {
                stat = EOK;
                break;
            }
            delay( 1 );
        }
    }

    return( stat );
}

#ifdef PXIV_SDMMC_DEBUG
static void pxiv_reg_dump( sdio_hc_t *hc, const char *func, int line )
{
    pxiv_sdmmc_hc_t *sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    uintptr_t       base = sdhc->base;
    uintptr_t       ctrl_base = sdhc->ctrl_base;

    sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: called from: %s() line %d", __func__, func, line );

    sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: Interface clock and reset control registers:", __func__ );

    sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: MODECTRL: 0x%X BYTESWAP: 0x%X CLKCTRL: 0x%X RSTCTRL: 0x%X HWRST: 0x%X",
                __func__,
                in32( ctrl_base + PXIV_SD_MODECTRL ), in32( ctrl_base + PXIV_SD_BYTESWAP ),
                in32( ctrl_base + PXIV_SD_CLKCTRL ), in32( ctrl_base + PXIV_SD_RSTCTRL ),
                in32( ctrl_base + PXIV_SD_EMMC1_HWRST ) );

    sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: HC registers:", __func__ );

    sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: CMD: 0x%X ARG: 0x%X STOP: 0x%X SECCNT: 0x%X "
                "RSP10: 0x%X RSP32: 0x%X RSP54: 0x%X RSP76: 0x%X", __func__,
                in32( base + PXIV_SD_CMD ), in32( base + PXIV_SD_ARG0 ),
                in32( base + PXIV_SD_STOP ), in32( base + PXIV_SD_SECCNT ),
                in32( base + PXIV_SD_RSP10 ), in32( base + PXIV_SD_RSP32 ),
                in32( base + PXIV_SD_RSP54 ), in32( base + PXIV_SD_RSP76 ) );

    sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: INFO1: 0x%X INFO2: 0x%X MASK1: 0x%X MASK2: 0x%X "
                "CLK_CTRL: 0x%X SIZE: 0x%X OPTION: 0x%X EXT MODE: 0x%X", __func__,
                in32( base + PXIV_SD_INFO1 ), in32( base + PXIV_SD_INFO2 ),
                in32( base + PXIV_SD_INFO1_MASK ), in32( base + PXIV_SD_INFO2_MASK ),
                in32( base + PXIV_SD_CLK_CTRL ), in32( base + PXIV_SD_SIZE ),
                in32( base + PXIV_SD_OPTION ), in32( base + PXIV_CC_EXT_MODE ) );

    sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: ERR_STS1: 0x%X ERR_STS2: 0x%X HOST MODE: 0x%X "
                "SDIF MODE: 0x%X STATUS: 0x%X", __func__,
                in32( base + PXIV_SD_ERR_STS1 ), in32( base + PXIV_SD_ERR_STS2 ),
                in32( base + PXIV_HOST_MODE ), in32( base + PXIV_SDIF_MODE ),
                in32( base + PXIV_SD_STATUS ) );

    sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: DMAC registers:", __func__ );

    sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: MODE 0x%X CTRL: 0x%X RST: 0x%X INFO1: 0x%X "
                "INFO2: 0x%X MASK1: 0x%X MASK2: 0x%X ADDR: 0x%X ADDREX: 0x%X", __func__,
                in32( base + PXIV_DM_CM_DTRAN_MODE ), in32( base + PXIV_DM_CM_DTRAN_CTRL ),
                in32( base + PXIV_DM_CM_RST ), in32( base + PXIV_DM_CM_INFO1 ),
                in32( base + PXIV_DM_CM_INFO2 ), in32( base + PXIV_DM_CM_INFO1_MASK ),
                in32( base + PXIV_DM_CM_INFO2_MASK ), in32( base + PXIV_DM_DTRAN_ADDR ),
                in32( base + PXIV_DM_DTRAN_ADDREX ) );
}
#endif

static int pxiv_pio_xfer( sdio_hc_t *hc, sdio_cmd_t *cmd )
{
    pxiv_sdmmc_hc_t *sdhc;
    int             len, msk, blksz, blks = cmd->blks;
    uint8_t         *addr;

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    msk = (cmd->flags & SCF_DIR_IN) ? PXIV_SD_INFO2_BRE : PXIV_SD_INFO2_BWE;

    while( blks-- ) {
        blksz = cmd->blksz;
        while( blksz ) {

            if( sdio_sg_nxt( hc, &addr, &len, blksz ) ) {
                break;
            }

            blksz -= len;
            len /= 4;

            // Wait until the read or write buffer is ready
            if( pxiv_waitmask( hc, PXIV_SD_INFO2, msk, msk, PXIV_TRANSFER_TIMEOUT ) ) {
                sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: pio read: timed out for buffer read", __func__ );
                return( ETIMEDOUT );
            } else {
                // Clear the BRE / BWE bits in case its a multiblock transfer
                out32( sdhc->base + PXIV_SD_INFO2, ~(PXIV_SD_INFO2_BWE | PXIV_SD_INFO2_BRE) );
            }

            if( ( cmd->flags & SCF_DIR_IN ) ) {
                in32s( addr, len, sdhc->base + PXIV_SD_BUF );
            }
            else {
                out32s( addr, len, sdhc->base + PXIV_SD_BUF );
            }
        }
    }
    return( EOK );
}

static int pxiv_sdma_setup( sdio_hc_t *hc, sdio_cmd_t *cmd )
{
    pxiv_sdmmc_hc_t *sdhc;
    sdio_sge_t      *sgp;
    int             sgc;

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;

    sgc = cmd->sgc;
    sgp = cmd->sgl;

    if( !( cmd->flags & SCF_DATA_PHYS ) ) {
        sdio_vtop_sg( sgp, sdhc->sgl, sgc, cmd->mhdl );
        sgp = sdhc->sgl;
    }

    // DMA mode
    out32_mask( sdhc->base + PXIV_CC_EXT_MODE, PXIV_CC_EXT_MODE_DMASDRW, PXIV_CC_EXT_MODE_DMASDRW );

    // Set the address mode
    out32( sdhc->base + PXIV_DM_CM_DTRAN_MODE,
           PXIV_DM_CM_DTRAN_MODE_WIDTH_64 | PXIV_DM_CM_DTRAN_MODE_ADDR_MODE |
           ((cmd->flags & SCF_DIR_IN) ? PXIV_DM_CM_DTRAN_MODE_CH_RX : PXIV_DM_CM_DTRAN_MODE_CH_TX) );

    // Set the SDMA address
    if( (sgp->sg_address & 0x7) ) {
        // The DMA has an 8 byte alignment requirement. Return an error so we'll fall back to PIO for this xfer
        out32_mask( sdhc->base + PXIV_CC_EXT_MODE, PXIV_CC_EXT_MODE_DMASDRW, 0 );
        return( EINVAL );
    }
    out32( sdhc->base + PXIV_DM_DTRAN_ADDR, sgp->sg_address );

    return( EOK );
}

static int pxiv_sdma_xfer( sdio_hc_t *hc, sdio_cmd_t *cmd )
{
    pxiv_sdmmc_hc_t *sdhc;

    uint32_t dir;

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;

    // Clear status
    out32( sdhc->base + PXIV_DM_CM_INFO1, 0);
    out32( sdhc->base + PXIV_DM_CM_INFO2, 1);

    // Umask error interrupt for channel (TX or RX)
    dir = ( cmd->flags & SCF_DIR_IN ) ? PXIV_DM_CM_INFO2_MASK_DTARERR_RX : PXIV_DM_CM_INFO2_MASK_DTARERR_TX;
    out32( sdhc->base + PXIV_DM_CM_INFO2_MASK, in32(sdhc->base + PXIV_DM_CM_INFO2_MASK) & ~dir );

    // Start DMA
    out32( sdhc->base + PXIV_DM_CM_DTRAN_CTRL, PXIV_DM_CM_DTRAN_CTRL_DM_START );

    return( EOK );
}

static int pxiv_sdma_cmplt( sdio_hc_t *hc )
{
    pxiv_sdmmc_hc_t *sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;

    // Cleanup DMA
    sdhc->flags &= ~SF_SDMA_ACTIVE;

    // Clear status
    out32( sdhc->base + PXIV_DM_CM_INFO1, 0);
    out32( sdhc->base + PXIV_DM_CM_INFO2, 1);

    // Mask interrupts
    out32( sdhc->base + PXIV_DM_CM_INFO1_MASK, PXIV_DM_CM_INFO1_MASK_ALL );
    out32( sdhc->base + PXIV_DM_CM_INFO2_MASK, PXIV_DM_CM_INFO2_MASK_ALL );

    // Disable DMA mode
    out32_mask( sdhc->base + PXIV_CC_EXT_MODE, PXIV_CC_EXT_MODE_DMASDRW, 0);

    return( EOK );
}

static int pxiv_reset( sdio_hc_t *hc, uint32_t rst )
{
    pxiv_sdmmc_hc_t *sdhc;

    sdio_slogf( _SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, _SLOG_INFO, "%s: rst: 0x%x", __func__, rst );

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;

    if ( rst & PXIV_IF_RESET ) {
        // Clear
        out32( sdhc->ctrl_base + PXIV_SD_RSTCTRL, PXIV_SD_RSTCTRL_SDIORST );
        delay(1);
        // Reset
        out32( sdhc->ctrl_base + PXIV_SD_RSTCTRL, 0 );
        delay(1);
        // Clear
        out32( sdhc->ctrl_base + PXIV_SD_RSTCTRL, PXIV_SD_RSTCTRL_SDIORST );
    }

    if ( rst & PXIV_HC_SOFT_RST ) {
        out32_mask( sdhc->base + PXIV_SOFT_RST, PXIV_SOFT_RST_SDRST, 0 );
        delay(1);
        out32_mask( sdhc->base + PXIV_SOFT_RST, PXIV_SOFT_RST_SDRST, PXIV_SOFT_RST_SDRST );
    }

    return( EOK );
}

static int intr_event( sdio_hc_t *hc )
{
    pxiv_sdmmc_hc_t *sdhc;
    sdio_cmd_t      *cmd;
    uint32_t        info1, info2, mask1, mask2;
    uint32_t        dma_info1, dma_info2;
    int             cs, idx;
    uint32_t        rsp;
    uint8_t         rbyte;

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    cs = CS_CMD_INPROG;

    mask1 = ~in32( sdhc->base + PXIV_SD_INFO1_MASK );
    mask2 = ~in32( sdhc->base + PXIV_SD_INFO2_MASK );

    info1 = in32( sdhc->base + PXIV_SD_INFO1 ) & mask1;
    info2 = in32( sdhc->base + PXIV_SD_INFO2 ) & mask2;
    dma_info1 = in32( sdhc->base + PXIV_DM_CM_INFO1 );
    dma_info2 = in32( sdhc->base + PXIV_DM_CM_INFO2 );

    // Clear interrupt status
    out32( sdhc->base + PXIV_SD_INFO1, ~info1 );
    out32( sdhc->base + PXIV_SD_INFO2, ~info2 );

#ifdef PXIV_SDMMC_DEBUG
    sdio_slogf( _SLOGC_SDIODI, _SLOG_DEBUG1, hc->cfg.verbosity, 1, "%s: info1: 0x%X info2: 0x%X mask1: 0x%X mask2: 0x%X",
                __func__, info1, info2, mask1, mask2 );

    sdio_slogf( _SLOGC_SDIODI, _SLOG_DEBUG1, hc->cfg.verbosity, 1, "%s: dma_info1: 0x%X dma_info2 0x%X mask1 0x%X mask2 0x%X",
                    __func__, dma_info1, dma_info2, in32( sdhc->base + PXIV_DM_CM_INFO1_MASK ),
                    in32( sdhc->base + PXIV_DM_CM_INFO2_MASK ) );
#endif

    if( ( cmd = hc->wspc.cmd ) == NULL ) {
        return( EOK );
    }

    // Card insertion and removal events
    if( info1 & (PXIV_SD_INFO1_INFO4_INSERT | PXIV_SD_INFO1_INFO3_REMOVE) ) {
        sdio_hc_event( hc, HC_EV_CD );
    }

    // DMA error processing
    if( (sdhc->flags & SF_SDMA_ACTIVE) && (dma_info2 & PXIV_DM_CM_INFO2_DTARERR_MASK) ) {
        sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: CMD%d DMA error DMA info1: 0x%X DMA info2 0x%X",
                            __func__, cmd->opcode, dma_info1, dma_info2 );
        // Reset DMA channels
        out32_mask( sdhc->base + PXIV_DM_CM_RST, PXIV_DM_CM_RST_DTRANRST_MASK, 0 );
        delay(1);
        out32_mask( sdhc->base + PXIV_DM_CM_RST, PXIV_DM_CM_RST_DTRANRST_MASK, PXIV_DM_CM_RST_DTRANRST_MASK );
        pxiv_sdma_cmplt( hc );
        cs = CS_CMD_CMP_ERR;
    }

    // Error checking
    if( info2 & PXIV_SD_INFO2_ALL_ERR ) {

        uint32_t ests1 = in32( sdhc->base + PXIV_SD_ERR_STS1 );

        if( info2 & PXIV_SD_INFO2_ERR3_DTO )                        cs = CS_DATA_TO_ERR;
        if( ests1 & PXIV_SD_ERR_STS1_E10 )                          cs = CS_DATA_CRC_ERR;
        if( ests1 & PXIV_SD_ERR_STS1_E4 )                           cs = CS_DATA_END_ERR;
        if( info2 & PXIV_SD_INFO2_ERR6_RTO )                        cs = CS_CMD_TO_ERR;
        if( ests1 & (PXIV_SD_ERR_STS1_E9 | PXIV_SD_ERR_STS1_E8) )   cs = CS_CMD_CRC_ERR;
        if( ests1 & (PXIV_SD_ERR_STS1_E3 | PXIV_SD_ERR_STS1_E2) )   cs = CS_CMD_END_ERR;
        if( ests1 & (PXIV_SD_ERR_STS1_E1 | PXIV_SD_ERR_STS1_E0) )   cs = CS_CMD_IDX_ERR;
        if( !cs )                                                   cs = CS_CMD_CMP_ERR;

    } else {
        if( info1 & PXIV_SD_INFO1_INFO0_CC ) {
            if( ( cmd->flags & SCF_RSP_136 ) ) {
                // CRC is not included in the response reg, left shift 8 bits to match the 128 CID/CSD structure
                for( idx = 0, rbyte = 0; idx < 4; idx++ ) {
                    rsp = in32( sdhc->base + PXIV_SD_RSP10 + idx * 8 );
                    cmd->rsp[3 - idx] = (rsp << 8) + rbyte;
                    rbyte = rsp >> 24;
                }
            }
            else if( ( cmd->flags & SCF_RSP_PRESENT ) ) {
                cmd->rsp[0] = in32( sdhc->base + PXIV_SD_RSP10 );
            }

            if( ( cmd->flags & SCF_DATA_MSK ) ) {
                if( sdhc->flags & SF_USE_SDMA  && (sdhc->flags & SF_SDMA_ACTIVE) ) {
                    if (pxiv_sdma_xfer( hc, cmd )) {
                        cs = CS_DATA_TO_ERR;
                    }
                } else {
                    if (pxiv_pio_xfer( hc, cmd )) {
                        cs = CS_DATA_TO_ERR;
                    }
                }
            } else {
                // CC & no data so mark as completed, otherwise we wait for the TC interrupt
                cs = CS_CMD_CMP;
            }
        }

        // End of data transfer
        if( ( info1 & PXIV_SD_INFO1_INFO2_TC ) ) {
            cs = CS_CMD_CMP;

            if( sdhc->flags & SF_SDMA_ACTIVE ) {
                pxiv_sdma_cmplt( hc );
            }
        }
    }

    if( cs != CS_CMD_INPROG ) {
        sdio_cmd_cmplt( hc, cmd, cs );
    }
    return( EOK );
}

static int pxiv_event( sdio_hc_t *hc, sdio_event_t *ev )
{
    int status = CS_CMD_INPROG;

    switch( ev->code ) {
        case HC_EV_INTR:
            status = intr_event( hc );
            InterruptUnmask( hc->cfg.irq[0], hc->hc_iid );
            break;
        default:
            break;
    }

    return( status );
}

static int pxiv_xfer_setup( sdio_hc_t *hc, sdio_cmd_t *cmd, uint32_t *command, uint32_t *imask, uint32_t *emask )
{
    pxiv_sdmmc_hc_t *sdhc;
    int status = 0;

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    status = EOK;

    // Enable data transfer
    *command |= PXIV_SD_CMD_MD3_ADTC;

    status = sdio_sg_start( hc, cmd->sgl, cmd->sgc );

    // Write the block size
    out32( sdhc->base + PXIV_SD_SIZE, cmd->blksz );

    if( cmd->sgc && ( hc->caps & HC_CAP_DMA ) ) {
        if( ( sdhc->flags & SF_USE_SDMA ) ) {
            if( ( status = pxiv_sdma_setup( hc, cmd ) ) == EOK ) {
                sdhc->flags |= SF_SDMA_ACTIVE;
            } else {
                // Fallback to PIO
                sdhc->flags &= ~SF_SDMA_ACTIVE;
                status = EOK;
            }
        }
    }

    if( cmd->blks > 1 ) {
        // Enable SECCNT
        out32( sdhc->base + PXIV_SD_STOP, in32( sdhc->base + PXIV_SD_STOP ) | PXIV_SD_STOP_SEC );

        // Set the SECCNT
        out32( sdhc->base + PXIV_SD_SECCNT, cmd->blks );

        // Multi block transfer
        *command |= PXIV_SD_CMD_MD5_BLK_MULT;
    } else {
        // Disable seccnt
        out32( sdhc->base + PXIV_SD_STOP, in32( sdhc->base + PXIV_SD_STOP ) & ~PXIV_SD_STOP_SEC );
    }

    if( cmd->flags & SCF_DIR_IN ) {
        *command |= PXIV_SD_CMD_MD4_READ;
    }

    return( status );
}

static int pxiv_cmd( sdio_hc_t *hc, sdio_cmd_t *cmd )
{
    pxiv_sdmmc_hc_t *sdhc;
    uint32_t        imask;
    uint32_t        emask;
    int             status;
    uint32_t        command = 0;

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    imask = PXIV_SD_INFO1_ALL_CMP;
    emask = PXIV_SD_INFO2_ALL_ERR;
    command = cmd->opcode & PXIV_SD_CMD_CF_MASK;

    // Wait until card is ready for the next command
    if( pxiv_waitmask( hc, PXIV_SD_INFO2, PXIV_SD_INFO2_CBUSY, 0, PXIV_COMMAND_TIMEOUT ) ) {
         sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0,
            "%s: timeout on cbusy for CMD%d ", __func__, cmd->opcode );
         return( ETIMEDOUT );
    }

    // Clear status
    out32( sdhc->base + PXIV_SD_INFO1, 0);
    out32( sdhc->base + PXIV_SD_INFO2, 0);

#ifdef PXIV_SDMMC_DEBUG
    sdio_slogf( _SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, _SLOG_INFO, "%s: cmd: %d flags: 0x%x arg: 0x%x blks: %d blksz: %d",
                __func__, cmd->opcode, cmd->flags, cmd->arg, cmd->blks, cmd->blksz );
#endif

    if( ( cmd->flags & SCF_DATA_MSK ) ) {
        if( ( cmd->blks && (status = pxiv_xfer_setup( hc, cmd, &command, &imask, &emask ) )) != EOK )
            return( status );
    }

    if( cmd->flags & SCF_RSP_PRESENT ) {
        switch( cmd->flags & 0x1F0 ) {
            case SCF_RSP_R1:
                command |= PXIV_SD_CMD_MD2_EXT_RESP_R1;
                break;
            case SCF_RSP_R1B:
                command |= PXIV_SD_CMD_MD2_EXT_RESP_R1B;
                break;
            case SCF_RSP_R2:
                command |= PXIV_SD_CMD_MD2_EXT_RESP_R2;
                break;
            case SCF_RSP_R3:
                command |= PXIV_SD_CMD_MD2_EXT_RESP_R3;
                break;
            case SCF_RSP_NONE:
                command |= PXIV_SD_CMD_MD2_EXT_NO_RESP;
                break;
            default:
                sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: unrecognized resp (flags: 0x%X) for CMD%d",
                            __func__, cmd->flags, cmd->opcode );
                return( ENOTSUP );
        }
    }

    if( cmd->flags & SCF_APP_CMD ) {
        command |= PXIV_SD_CMD_C_APP_ACMD;
    }

#ifdef PXIV_SDMMC_DEBUG
    // CMD reg value
    sdio_slogf( _SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, _SLOG_INFO, "%s: command: 0x%x", __func__, command );
#endif

    out32( sdhc->base + PXIV_SD_INFO1_MASK, in32( sdhc->base + PXIV_SD_INFO1_MASK ) & ~imask );
    out32( sdhc->base + PXIV_SD_INFO2_MASK, in32( sdhc->base + PXIV_SD_INFO2_MASK ) & ~emask );
    out32( sdhc->base + PXIV_SD_ARG0, cmd->arg );
    out32( sdhc->base + PXIV_SD_CMD, command );

    return( EOK );
}

static int pxiv_abort( sdio_hc_t *hc, sdio_cmd_t *cmd )
{
    return( EOK );
}

static int pxiv_pwr( sdio_hc_t *hc, int vdd )
{
    pxiv_sdmmc_hc_t *sdhc;
    int supply = 0;

    sdio_slogf( _SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, _SLOG_INFO, "%s: vdd 0x%x", __func__, vdd );

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;

    if( vdd ) {
        if( vdd < OCR_VDD_27_28) {
            supply = PXIV_SD_STATUS_PWR_1V8;
        }
        else {
            supply = PXIV_SD_STATUS_PWR_3V3;
        }
    }
    else {
        supply = PXIV_SD_STATUS_PWR_OFF;
    }

    out32( sdhc->base + PXIV_SD_STATUS, supply );
    delay(1);
    hc->vdd = vdd;
    return( EOK );
}

static int pxiv_bus_mode( sdio_hc_t *hc, int bus_mode )
{
    sdio_slogf( _SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, _SLOG_INFO, "%s: bus_mode %d", __func__, bus_mode );
    hc->bus_mode = bus_mode;
    return( EOK );
}

static int pxiv_bus_width( sdio_hc_t *hc, int width )
{
    pxiv_sdmmc_hc_t *sdhc;
    uintptr_t base;
    uint32_t reg;

    sdio_slogf( _SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, _SLOG_INFO, "%s: width %d", __func__, width );
    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    base = sdhc->base;
    reg = in32( base + PXIV_SD_OPTION ) & ~(PXIV_SD_OPTION_WIDTH | PXIV_SD_OPTION_WIDTH8);

    switch( width ) {
        case BUS_WIDTH_8:
            reg |= PXIV_SD_OPTION_WIDTH8;
            break;

        case BUS_WIDTH_4:
            // Both bits off, already cleared after reading the register above
            break;

        case BUS_WIDTH_1:
        default:
            reg |= PXIV_SD_OPTION_WIDTH;
            break;
    }

    out32( base + PXIV_SD_OPTION, reg);
    hc->bus_width = width;
    return( EOK );
}

static int pxiv_clk( sdio_hc_t *hc, int clk )
{
    pxiv_sdmmc_hc_t *sdhc;
    uint32_t         sctl;
    uint32_t         div = 1;
    uint32_t         clkrt;

    sdio_slogf( _SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, _SLOG_INFO, "%s clk: %d", __func__, clk );

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    sctl = in32( sdhc->base + PXIV_SD_CLK_CTRL );

    // Stop clock
    out32( sdhc->base + PXIV_SD_CLK_CTRL, sctl & ~PXIV_SD_CLK_CTRL_SCLKEN );

    // The max possible clock rate is 100 MHz as that's the interface clock freq. However to run at higher > 50 MHz
    // (i.e. SDR50), the SDCLKSEL bit has to be set to bypass the divider in the clock control reg & the SDR bit in
    // the interace mode control reg has to be enabled (along with selecting the correct interface clock). This is
    // currently not implemented so the max supported rate is 50 MHz (all divider bits off)
    if ( clk > hc->clk_max ) {
        clk = hc->clk_max;
    }

    while ( (hc->clk_max / div) > clk && (div <= 512) ) {
        div <<= 1;
    }

    // Divisor bit 8 is disjoint from bits [7:0] in the register (i.e. it's bit 16)
    if (div == 512) {
        clkrt = PXIV_SD_CLK_CTRL_DIV8;
    } else {
        clkrt = div >> 1;
    }

    sctl = PXIV_SD_CLK_CTRL_SCLKEN | (clkrt & 0xff);

    sdio_slogf( _SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, _SLOG_INFO, "%s: actual SD clk: %d div: %d clkrt: 0x%x",
                __func__, hc->clk_max / div, div, clkrt);

    out32( sdhc->base + PXIV_SD_CLK_CTRL, sctl );
    hc->clk = hc->clk_max / div;

    return( EOK );
}

static int pxiv_timing( sdio_hc_t *hc, int timing )
{
    pxiv_sdmmc_hc_t *sdhc;
    uint32_t sdif_mode, clk_ctrl;

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    sdif_mode = in32( sdhc->base + PXIV_SDIF_MODE ) & ~PXIV_SDIF_MODE_DDR;
    clk_ctrl = in32( sdhc->base + PXIV_SD_CLK_CTRL ) & ~PXIV_SD_CLK_CTRL_SDCLKSEL;

    sdio_slogf( _SLOGC_SDIODI, _SLOG_INFO, hc->cfg.verbosity, _SLOG_INFO, "%s: timing: %d", __func__, timing );

    switch( timing ) {
        case TIMING_DDR50:
            sdif_mode |= PXIV_SDIF_MODE_DDR;
            break;
        case TIMING_SDR50:
        case TIMING_SDR25:
        case TIMING_HS:
            // SD_CLK_CTRL register's SDCLKSEL bit says it should be set for HS, SDR25, SDR50 & that the SDCLK
            // will be equal to the interface clock
            clk_ctrl |= PXIV_SD_CLK_CTRL_SDCLKSEL;
            break;
        default:
            break;
    }

    out32 ( sdhc->base + PXIV_SDIF_MODE, sdif_mode );
    out32 ( sdhc->base + PXIV_SD_CLK_CTRL, clk_ctrl );
    hc->timing = timing;
    return( EOK );
}

static int pxiv_signal_voltage( sdio_hc_t *hc, int signal_voltage )
{
    // The HC does not have seperate controls for the supply and IO voltage levels
    return( EOK );
}

static int pxiv_cd( sdio_hc_t *hc )
{
    pxiv_sdmmc_hc_t *sdhc;
    uint32_t pstate, cstate;

    sdio_slogf( _SLOGC_SDIODI, _SLOG_DEBUG1, hc->cfg.verbosity, _SLOG_DEBUG1, "%s: caps: 0x%llx ", __func__, hc->caps );
    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;

    cstate = CD_RMV;
    pstate = in32( sdhc->base + PXIV_SD_INFO1 );

    if( !(hc->caps & HC_CAP_CD_INTR) ) {
        // emmc or nocd option specified to bs layer, assume card is always inserted
        return CD_INS;
    }

    if( pstate & PXIV_SD_INFO1_INFO5_CD ) {
         cstate |= CD_INS;

         // Test write protection
         if ( pstate & PXIV_SD_INFO1_INFO7_WP ){
             cstate |= CD_WP;
         }
     }

    return( cstate );
}

int pxiv_dinit( sdio_hc_t *hc )
{
    pxiv_sdmmc_hc_t *sdhc;

    if( !hc || !hc->cs_hdl)
        return( EOK );

    sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;

    if( sdhc->base ) {
        pxiv_pwr( hc, 0 );
        pxiv_reset( hc, PXIV_IF_RESET );
        if( hc->hc_iid != -1 ) {
            InterruptDetach( hc->hc_iid );
        }
        munmap_device_memory( (void *)sdhc->base, hc->cfg.base_addr_size[0] );

        if( sdhc-> ctrl_base ) {
            munmap_device_memory( (void *)sdhc->ctrl_base, PXIV_SDHC_CTL_SIZE);
        }
    }

    free( sdhc );
    hc->cs_hdl = NULL;

    return( EOK );
}

static void pxiv_intf_init( sdio_hc_t *hc )
{
    pxiv_sdmmc_hc_t *sdhc = (pxiv_sdmmc_hc_t *)hc->cs_hdl;
    sdio_hc_cfg_t *cfg = &hc->cfg;
    uint32_t mode = 0;

    // Disable SD clock
    out32_mask( sdhc->base + PXIV_SD_CLK_CTRL, PXIV_SD_CLK_CTRL_SCLKEN, 0 );

    // Enable interface clock
    out32( sdhc->ctrl_base + PXIV_SD_CLKCTRL, PXIV_SD_CLKCTRL_SDIO_ON );

    // Reset the interface
    pxiv_reset( hc, PXIV_IF_RESET );

    // Enable 32bit access to SD_BUF
    out32( sdhc->base + PXIV_HOST_MODE, PXIV_HOST_MODE_BUSWIDTH | PXIV_HOST_MODE_WMODE );

    // Set mode (TRM says interface 1 is for eMMC and interface 2 for SD, are they interchangeable?) & the
    // interface clock frequency to 100 MHz (max)
    if( cfg->base_addr[0] == PXIV_SDHC0_BASE ) {
        mode |= PXIV_SD_MODECTRL_MMCMODE;
    } else {
        mode &= ~PXIV_SD_MODECTRL_MMCMODE;
    }
    out32( sdhc->ctrl_base + PXIV_SD_MODECTRL, mode | PXIV_SD_MODECTRL_CLKMODE );

    // Soft reset
    pxiv_reset( hc, PXIV_HC_SOFT_RST );

    // Mask all interrupts
    out32( sdhc->base + PXIV_SD_INFO1_MASK, PXIV_SD_INFO1_MASK_ALL );
    out32( sdhc->base + PXIV_SD_INFO2_MASK, PXIV_SD_INFO2_MASK_ALL );
}

int pxiv_init( sdio_hc_t *hc )
{
    sdio_hc_cfg_t   *cfg;
    pxiv_sdmmc_hc_t *sdhc;

    uintptr_t base;
    uintptr_t ctrl_base;
    struct sigevent event;

    hc->hc_iid = -1;
    cfg = &hc->cfg;

    memcpy( &hc->entry, &pxiv_sdmmc_hc_entry, sizeof( sdio_hc_entry_t ) );

    if( !cfg->base_addr[0] || !cfg->base_addr[1] ) {
        sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 0, "%s: base addresses not set", __func__ );
        return( ENODEV );
    }

    if( !cfg->base_addr_size[0] ) {
        cfg->base_addr_size[0] = PXIV_SDHC_SIZE;
    }

    if( !cfg->base_addr_size[1] ) {
        cfg->base_addr_size[1] = PXIV_SDHC_CTL_SIZE;
    }

#ifdef PXIV_SDMMC_DEBUG
    sdio_slogf( _SLOGC_SDIODI, _SLOG_DEBUG1, hc->cfg.verbosity, _SLOG_DEBUG1, "%s: BA0: 0x%llx BA1: 0x%llx",
                __func__, cfg->base_addr[0], cfg->base_addr[1]);
#endif

    if( ( sdhc = hc->cs_hdl = calloc( 1, sizeof( pxiv_sdmmc_hc_t ) ) ) == NULL ) {
        return( ENOMEM );
    }

    if( ( base = sdhc->base = (uintptr_t)mmap_device_io( cfg->base_addr_size[0], cfg->base_addr[0] ) ) == (uintptr_t)MAP_FAILED ) {
        sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: SDHCI base mmap_device_io (0x%llx) %s",
                    __func__, cfg->base_addr[0], strerror( errno ) );
        pxiv_dinit( hc );
        return( errno );
    }

    if( ( ctrl_base = sdhc->ctrl_base = (uintptr_t)mmap_device_io( cfg->base_addr_size[1], cfg->base_addr[1] ) ) == (uintptr_t)MAP_FAILED ) {
        sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1, "%s: SDHCI ctrl_reg base mmap_device_io (0x%llx) %s",
                    __func__, cfg->base_addr[1], strerror( errno ) );
        pxiv_dinit( hc );
        return( errno );
    }

    if( cfg->clk && (cfg->clk < PXIV_CLOCK_DEFAULT) ) {
        hc->clk_max = cfg->clk;
    } else {
        hc->clk_max = PXIV_CLOCK_DEFAULT;
    }
    hc->clk_min = (PXIV_CLOCK_DEFAULT / 512);

    // The HC has no capability regs
    hc->caps |= HC_CAP_BW4 | HC_CAP_BW8 | HC_CAP_BSY | HC_CAP_CD_INTR;
    hc->caps |= HC_CAP_ACMD12 | HC_CAP_HS;
    hc->caps |= HC_CAP_SDR50 | HC_CAP_SDR25 | HC_CAP_SDR12;
    hc->caps |= HC_CAP_DDR50;

    hc->ocr = OCR_VDD_17_195 | OCR_VDD_32_33 | OCR_VDD_33_34;
    hc->caps |= HC_CAP_SV_3_3V | HC_CAP_SV_1_8V;

#ifdef PXIV_DMA_SUPPORTED
    hc->caps |= HC_CAP_DMA;
    hc->cfg.sg_max  = 1;
    sdhc->flags |= SF_USE_SDMA;
#endif

    hc->caps &= cfg->caps;       // reconcile command line options

    // Setup interface control registers (i.e. interface clock, mode, interface reset)
    pxiv_intf_init( hc );

    SIGEV_PULSE_INIT( &event, hc->hc_coid, SDIO_PRIORITY, HC_EV_INTR, NULL );
    if( ( hc->hc_iid = InterruptAttachEvent( cfg->irq[0], &event, _NTO_INTR_FLAGS_TRK_MSK ) ) == -1 ) {
        sdio_slogf( _SLOGC_SDIODI, _SLOG_ERROR, hc->cfg.verbosity, 1,
                    "%s: InterrruptAttachEvent (irq 0x%x) - %s", __func__, cfg->irq[0], strerror( errno ) );
        pxiv_dinit( hc );
        return( errno );
    }

    return( EOK );
}

#endif /* SDIO_HC_PXIV */

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/devb/sdmmc/sdiodi/hc/pxiv.c $ $Rev: 793476 $")
#endif
