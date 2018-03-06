/*
 * $QNXLicenseC: 
 * Copyright 2009, QNX Software Systems.  
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


#ifndef __MXESAI_H
#define __MXESAI_H

/*
 *  Definitions for i.MX ESAI audio controller
 */

struct   mxesai_card;
struct   mxesai_stream;
#define  HW_CONTEXT_T struct mxesai_card
#define  PCM_SUBCHN_CONTEXT_T   struct mxesai_stream

#include <audio_driver.h>
#include <hw/dma.h>
#include <string.h>
#include <proto.h>
#include <sys/hwinfo.h>
#include <drvr/hwinfo.h>
#include <variant.h>

typedef volatile uint32_t vuint32_t;
typedef volatile uint16_t vuint16_t;
typedef volatile uint8_t  vuint8_t;

#define FIFO_WATERMARK    4

typedef struct esai
{
    vuint32_t   etdr;
    vuint32_t   erdr;
    vuint32_t   ecr;
#define ECR_ETI    (1<<19)
#define ECR_ETO    (1<<18)
#define ECR_ERI    (1<<17)
#define ECR_ERO    (1<<16)
#define ECR_ERST   (1<<1)
#define ECR_ESAIEN (1<<0)
    vuint32_t   esr;
#define ESR_TINIT  (1<<10)
#define ESR_RFF    (1<<9)
#define ESR_TFE    (1<<8)
#define ESR_TLS    (1<<7)
#define ESR_TDE    (1<<6)
#define ESR_TED    (1<<5)
#define ESR_TD     (1<<4)
#define ESR_RLS    (1<<3)
#define ESR_RDE    (1<<2)
#define ESR_RED    (1<<1)
#define ESR_RD     (1<<0)
    vuint32_t   tfcr;
#define TFCR_TIEN    (1<<19)
#define TFCR_TWA(x)  ((x&7)<<16)
#define TFCR_TFWM(x) ((x&0xff)<<8)
#define TFCR_TE(x)   ((1<<(x+2))&0xfc)
#define TFCR_TEMASK  (0xfc)
#define TFCR_TFR     (1<<1)
#define TFCR_TFE     (1<<0)
    vuint32_t   tfsr;
#define TFSR_TFCNT(x) (x&0xff) 
    vuint32_t   rfcr;
#define RFCR_REXT     (1<<19)
#define RFCR_RWA(x)   ((x&7)<<16)
#define RFCR_RFWM(x)  ((x&0xff)<<8)
#define RFCR_RE(x)    ((1<<(x+2))&0x3c)
#define RFCR_REMASK   (0x3c)
#define RFCR_RFR      (1<<1)
#define RFCR_RFE      (1<<0)
    vuint32_t   rfsr;
#define RFCR_RFCNT(x) (x&0xff)
    vuint32_t   rsvd0[24];
    vuint32_t   tx0;
    vuint32_t   tx1;
    vuint32_t   tx2;
    vuint32_t   tx3;
    vuint32_t   tx4;
    vuint32_t   tx5;
    vuint32_t   tsr;
    vuint32_t   rsvd1;
    vuint32_t   rx0;
    vuint32_t   rx1;
    vuint32_t   rx2;
    vuint32_t   rx3;
    vuint32_t   rsvd2[7];
    vuint32_t   saisr;
#define SAISR_TDE   (1<<15)
#define SAISR_TUE   (1<<14)
    vuint32_t   saicr;
#define SAICR_ALC   (1<<8)
#define SAICR_SYNC  (1<<6)
    vuint32_t   tcr;
#define TCR_TPR     (1<<19)
#define TCR_PADC    (1<<17)
#define TCR_TFSR    (1<<16)
#define TCR_TFSL    (1<<15)
#define TCR_TSWS(x) ((x&0x1f)<<10)
#define TCR_TMOD(x) ((x&3)<<8)
#define TCR_TE(x)   ((1<<x)&0x3f)
#define TCR_TEMASK  0x3f
    vuint32_t   tccr;
#define TCCR_THCKD  (1<<23)
#define TCCR_TFSD   (1<<22)
#define TCCR_TCKD   (1<<21)
#define TCCR_THCKP  (1<<20)
#define TCCR_TFSP   (1<<19)
#define TCCR_TCKP   (1<<18)
#define TCCR_TFP(x) ((x&0xf)<<14)
#define TCCR_TDC(x) ((x&0x1f)<<9)
#define TCCR_TPSR   (1<<8)
#define TCCR_TPM(x) (x&0xff)
    vuint32_t   rcr;
#define RCR_RPR     (1<<19)
#define RCR_RFSR    (1<<16)
#define RCR_RFSL    (1<<15)
#define RCR_RSWS(x) ((x&0x1f)<<10)
#define RCR_RMOD(x) ((x&3)<<8)
#define RCR_RE(x)   ((1<<(x))&0xf)
#define RCR_REMASK  0xf
    vuint32_t   rccr;
#define RCCR_RHCKD   (1<<23)
#define RCCR_RFSD    (1<<22)
#define RCCR_RCKD    (1<<21)
#define RCCR_RHCKP   (1<<20)
#define RCCR_RFSP    (1<<19)
#define RCCR_RCKP    (1<<18)
#define RCCR_RFP(x)  ((x&0x1f)<<14)
#define RCCR_RDC(x)  ((x&0x1f)<<9)
#define RCCR_RPSP    (1<<8)
#define RCCR_RPM(x)  (x&0xff)
    vuint32_t   tsma;
    vuint32_t   tsmb;
    vuint32_t   rsma;
    vuint32_t   rsmb;
    vuint32_t   rsvd3;
    vuint32_t   prrc;
    vuint32_t   pcrc;
#define PORT0_SCKR       (1<<0)
#define PORT1_FSR        (1<<1)
#define PORT2_HCKR       (1<<2)
#define PORT3_SCKT       (1<<3)
#define PORT4_FST        (1<<4)
#define PORT5_HCKT       (1<<5)
#define PORT6_SDO5_SDI0  (1<<6)
#define PORT7_SDO4_SDI1  (1<<7)
#define PORT8_SDO3_SDI2  (1<<8)
#define PORT9_SDO2_SDI3  (1<<9)
#define PORT10_SDO1      (1<<10)
#define PORT11_SDO0      (1<<11)

} esai_t;

#define PLAY           2
#define RECORD         3

struct mxesai_stream {
    ado_pcm_subchn_t   *pcm_subchn;
    ado_pcm_cap_t      pcm_caps;
    ado_pcm_hw_t       pcm_funcs;
    ado_pcm_config_t   *pcm_config;
    int                voices;
    int                active;
    dma_addr_t         dma_buf;
    struct sigevent    sdma_event;
    void               *pulse;
    void               *dma_chn;
    uint32_t           pcm_offset;
};

typedef struct mxesai_stream mxesai_strm_t;

typedef struct mxesai_audio_interface
{
    ado_pcm_t           *pcm;
    mxesai_strm_t       play_strm;
    mxesai_strm_t       cap_strm;
}mxesai_aif_t;

#ifndef MAX_AIF
#define MAX_AIF 2
#endif

struct mxesai_card {
    ado_mutex_t         hw_lock;
    ado_mixer_t         *mixer;
    esai_t              *esai;
    dma_functions_t     sdmafuncs;
    mxesai_aif_t        *aif;
#define MAX_MIXEROPT 100
    char                mixeropts[MAX_MIXEROPT];      /* Mixer Specific Options */
    int                 i2c_bus;
    int                 i2c_addr;
    int                 sample_rate;
    int                 sample_rate_min;
    int                 sample_rate_max;
    uint32_t            esaibase;
    unsigned            tevt;
    unsigned            tchn;
    unsigned            revt;
    unsigned            rchn;
    int32_t             main_clk_src;
    int32_t             main_clk_freq;
    uint32_t            clk_mode;
    uint32_t            nslots;
    uint32_t            slot_size;
    int32_t             num_tx_aif;
    int32_t             num_rx_aif;
    int                 protocol;
    int                 bit_delay;
    int                 fsync_pol;
    int                 xclk_pol;
    int                 rclk_pol;
#define FS_WORD     0
#define FS_BIT      1
    int                 fs_active_width;
    int                 log_enabled;
    int                 play_log_fd;
    int                 cap_log_fd;
    uint32_t            tx_frag_size;
    uint32_t            rx_frag_size;
    vuint32_t           playback_frag_index;
    vuint32_t           capture_frag_index;
    ado_pcm_dmabuf_t    capture_dmabuf;
    ado_pcm_dmabuf_t    playback_dmabuf;
    int16_t             sdo_pins;             /* list of output pins stored as a bit map */
    int16_t             sdi_pins;             /* list of input pins stored as a bit map */
#define MAX_NUM_SDO_PINS    6
    int16_t             num_sdo_pins;
#define MAX_NUM_SDI_PINS    4
    int16_t             num_sdi_pins;
    int16_t             *sdo_reconstitute_buffer;
    int16_t             *sdi_reconstitute_buffer;
    int                 *sdo_chmap;
    int                 *sdi_chmap;
};

#define ESAI_CLK_SLAVE      0
#define ESAI_CLK_MASTER     1

#define PROTOCOL_I2S        0
#define PROTOCOL_PCM        1

#define MAIN_CLK_SRC_XTAL   0	/* On-chip clk from esai_clk_root in CCM */
#define MAIN_CLK_SRC_FSYS   1	/* On-chip system clock */
#define MAIN_CLK_SRC_OSC    2	/* External clk via HCKT pin */

#define ESAI_FIXED_DIV_2    2

typedef struct mxesai_card mxesai_t;

#endif	

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/deva/ctrl/mxesai/mxesai.h $ $Rev: 808011 $")
#endif
