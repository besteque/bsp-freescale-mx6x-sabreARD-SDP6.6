/*
 * $QNXLicenseC: 
 * Copyright 2010, 2011, 2013 QNX Software Systems.  
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
 
/*
 *
 *    mxesai_dll.c
 *      The primary interface into the mxesai DLL.
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <hw/inout.h>
#include <sys/asoundlib.h>
#include <devctl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mxesai.h"

#define SAMPLE_SIZE 2 /* Sample size in bytes */

/* Wait time count by usecond */
#define MAX_WAITTIME_US    1000

int codec_mixer (ado_card_t * card, HW_CONTEXT_T * hwc, ado_pcm_t *pcm);
int mxesai_set_clock_rate ( HW_CONTEXT_T *mx, int rate);
int32_t mxesai_playback_reconstitute (HW_CONTEXT_T *mx, PCM_SUBCHN_CONTEXT_T *pc, int8_t *dmaptr, size_t size);
int32_t mxesai_capture_reconstitute (HW_CONTEXT_T *mx, PCM_SUBCHN_CONTEXT_T *pc, int8_t *dmaptr, size_t size);

/**
 * This function returns the number of active capture channels
 */
static uint32_t
num_active_capture_interfaces(HW_CONTEXT_T * mx)
{
	uint32_t num; /* number of capture interfaces */
	uint32_t idx = 0;

	num = 0;
	for(idx=0; idx < mx->num_rx_aif; idx++)
	{
		if(mx->aif[idx].cap_strm.active == 1)
			num++;
	}
	return num;
}

/**
 * This function returns the number of active playback channels
 */
static uint32_t
num_active_playback_interfaces(HW_CONTEXT_T * mx)
{
	uint32_t num; /* number of playback interfaces */
	uint32_t idx = 0;

	num = 0;
	for(idx=0; idx < mx->num_tx_aif; idx++)
	{
		if(mx->aif[idx].play_strm.active == 1)
			num++;
	}
	return num;
}

int32_t
mxesai_capabilities (HW_CONTEXT_T * mx, ado_pcm_t *pcm, snd_pcm_channel_info_t * info)
{
	int   i;
	int   chn_avail = 1;

	if (info->channel == SND_PCM_CHANNEL_PLAYBACK)
	{
		info->fragment_align = SAMPLE_SIZE * mx->nslots * mx->num_sdo_pins;
		for (i = 0; i < mx->num_tx_aif; i++)
		{
			if (pcm == mx->aif[i].pcm && mx->aif[i].play_strm.pcm_subchn != NULL)
				chn_avail = 0;
		}

		if (chn_avail && (mx->clk_mode == ESAI_CLK_MASTER) && (mx->sample_rate_min != mx->sample_rate_max))
		{
			ado_mutex_lock(&mx->hw_lock);
			/* Playback and Capture are Rate locked, so adjust rate capabilities
			 * if the other side has an active stream.
			 */
			if (num_active_capture_interfaces(mx) != 0)
			{
				info->min_rate = info->max_rate = mx->sample_rate;
				info->rates = ado_pcm_rate2flag(mx->sample_rate);
			}
			ado_mutex_unlock(&mx->hw_lock);
		}
	}
	else if (info->channel == SND_PCM_CHANNEL_CAPTURE)
	{
		info->fragment_align = SAMPLE_SIZE * mx->nslots * mx->num_sdi_pins;
		for (i = 0; i < mx->num_rx_aif; i++)
		{
			if ( pcm == mx->aif[i].pcm && mx->aif[i].cap_strm.pcm_subchn != NULL)
				chn_avail = 0;
		}

		if (chn_avail && (mx->clk_mode == ESAI_CLK_MASTER) && (mx->sample_rate_min != mx->sample_rate_max))
		{
			ado_mutex_lock(&mx->hw_lock);
			/* Playback and Capture are Rate locked, so adjust rate capabilities
			 * if the other side has an active stream.
			 */
			if (num_active_playback_interfaces(mx) != 0)
			{
				info->min_rate = info->max_rate = mx->sample_rate;
				info->rates = ado_pcm_rate2flag(mx->sample_rate);
			}

			ado_mutex_unlock(&mx->hw_lock);
		}

	}

	if (chn_avail == 0)
	{
		info->formats = 0;
		info->rates = 0;
		info->min_rate = 0;
		info->max_rate = 0;
		info->min_voices = 0;
		info->max_voices = 0;
		info->min_fragment_size = 0;
		info->max_fragment_size = 0;
	}

	return (0);
}

int32_t
mxesai_playback_aquire (HW_CONTEXT_T * mx, PCM_SUBCHN_CONTEXT_T ** strm,
	ado_pcm_config_t * config, ado_pcm_subchn_t * subchn, uint32_t * why_failed)
{
	int i;
	mxesai_aif_t *aif = NULL;

	ado_mutex_lock (&mx->hw_lock);
	for ((*strm) = NULL, i = 0; i < mx->num_tx_aif; i++)
	{
		if (ado_pcm_subchn_is_channel(subchn, mx->aif[i].pcm, ADO_PCM_CHANNEL_PLAYBACK))
		{
			aif = &mx->aif[i];
			*strm = &aif->play_strm;
			break;
		}
	}

	if ((*strm) == NULL || (*strm)->pcm_subchn || aif == NULL)
	{
		*why_failed = SND_PCM_PARAMS_NO_CHANNEL;
		ado_mutex_unlock (&mx->hw_lock);
		return (EAGAIN);
	}

	(*strm)->active = 0;

	/* If on-the-fly sample rate switching is supported check if a rate switch is needed */
	if ((mx->clk_mode & ESAI_CLK_MASTER) && mx->sample_rate_min != mx->sample_rate_max)
	{
		if (config->format.rate != mx->sample_rate)
		{
			/* Playback and capture are rate locked, so wait until any active captures streams complete before
			 * changing the sample rate.
			 */
			if (num_active_capture_interfaces(mx) > 0)
			{
				ado_mutex_unlock(&mx->hw_lock);
				return (EBUSY);			
			}
			mxesai_set_clock_rate(mx, config->format.rate);
		}
	}

	config->dmabuf.flags = ADO_BUF_CACHE;
	/* If num_tx_aif == 1, then we DMA right from the pcm buffer so make it DMA safe */
	if (mx->num_tx_aif == 1)
		config->dmabuf.flags |= ADO_SHM_DMA_SAFE;
	if (ado_pcm_buf_alloc(config, config->dmabuf.size, config->dmabuf.flags) == NULL)
	{
		ado_mutex_unlock(&mx->hw_lock);
		return (errno);
	}

	if (mx->num_tx_aif == 1)
	{
		dma_transfer_t tinfo;
		int frag_idx;
	
		memset (&tinfo, 0, sizeof (tinfo));
		tinfo.src_addrs = malloc (config->mode.block.frags_total * sizeof (dma_addr_t));
	
		for (frag_idx = 0; frag_idx < config->mode.block.frags_total; frag_idx++)
		{
			tinfo.src_addrs[frag_idx].paddr =
				config->dmabuf.phys_addr + (frag_idx * config->mode.block.frag_size);
			tinfo.src_addrs[frag_idx].len = config->mode.block.frag_size;
		}
		tinfo.src_fragments  = config->mode.block.frags_total;
		tinfo.xfer_unit_size = 16;                             /* 16-bit samples */
		tinfo.xfer_bytes     = config->dmabuf.size;

		mx->sdmafuncs.setup_xfer ((*strm)->dma_chn, &tinfo);
		free (tinfo.src_addrs);
	}

	(*strm)->pcm_subchn = subchn;
	(*strm)->pcm_config = config;
	ado_mutex_unlock (&mx->hw_lock);
	return (EOK);
}


int32_t
mxesai_playback_release (HW_CONTEXT_T * mx, PCM_SUBCHN_CONTEXT_T * strm,
	ado_pcm_config_t * config)
{
	ado_mutex_lock (&mx->hw_lock);
	ado_pcm_buf_free(config);
	strm->pcm_subchn = NULL;
	strm->pcm_config = NULL;
	ado_mutex_unlock (&mx->hw_lock);
	return (0);
}

int32_t
mxesai_capture_aquire (HW_CONTEXT_T * mx, PCM_SUBCHN_CONTEXT_T ** strm,
	ado_pcm_config_t * config, ado_pcm_subchn_t * subchn, uint32_t * why_failed)
{
	int i;
	mxesai_aif_t *aif = NULL;

	ado_mutex_lock (&mx->hw_lock);
	for ((*strm) = NULL, i = 0; i < mx->num_rx_aif; i++)
	{
		if (ado_pcm_subchn_is_channel(subchn, mx->aif[i].pcm, ADO_PCM_CHANNEL_CAPTURE))
		{
			aif = &mx->aif[i];
			*strm = &aif->cap_strm;
			break;
		}
	}

	if ((*strm) == NULL || (*strm)->pcm_subchn || aif == NULL)
	{
		*why_failed = SND_PCM_PARAMS_NO_CHANNEL;
		ado_mutex_unlock(&mx->hw_lock);
		return EAGAIN;
	}

	(*strm)->active = 0;

	/* If multiple rates supported check for rate switch */
	if ((mx->clk_mode & ESAI_CLK_MASTER) && mx->sample_rate_min != mx->sample_rate_max)
	{
		if (config->format.rate != mx->sample_rate)
		{
			/* Playback and capture are rate locked, so wait until any active playback streams complete before
			 * changing the sample rate.
			 */
			if (num_active_playback_interfaces(mx) > 0)
			{
				ado_mutex_unlock(&mx->hw_lock);
				return (EBUSY);
			}
			mxesai_set_clock_rate(mx, config->format.rate);
		}
	}

	config->dmabuf.flags = ADO_BUF_CACHE;
	/* If num_rx_aif == 1, then we DMA right into the pcm buffer so make it DMA safe */
	if (mx->num_rx_aif == 1)
		config->dmabuf.flags |= ADO_SHM_DMA_SAFE;
	if (ado_pcm_buf_alloc(config, config->dmabuf.size, config->dmabuf.flags) == NULL)
	{
		ado_mutex_unlock(&mx->hw_lock);
		return (errno);
	}

	/* Only setup the DMA tranfer if there is only 1 capture interface/device.
	 * If multiple capture devices then a ping-pong DMA buffer will be allocated and initialized in ctrl_init().
	 */
	if (mx->num_rx_aif == 1)
	{
		dma_transfer_t tinfo;
		int frag_idx;

		memset (&tinfo, 0, sizeof (tinfo));
		tinfo.dst_addrs = malloc (config->mode.block.frags_total * sizeof (dma_addr_t));
		for (frag_idx = 0; frag_idx < config->mode.block.frags_total; frag_idx++)
		{
			tinfo.dst_addrs[frag_idx].paddr =
				config->dmabuf.phys_addr + (frag_idx * config->mode.block.frag_size);
			tinfo.dst_addrs[frag_idx].len = config->mode.block.frag_size;
		}
		tinfo.dst_fragments = config->mode.block.frags_total;
		tinfo.xfer_unit_size = 16;
		tinfo.xfer_bytes = config->dmabuf.size;

		mx->sdmafuncs.setup_xfer (mx->aif[0].cap_strm.dma_chn, &tinfo);
		free (tinfo.dst_addrs);
	}

	(*strm)->pcm_config = config;
	(*strm)->pcm_subchn = subchn;
	ado_mutex_unlock (&mx->hw_lock);
	return (EOK);
}

int32_t
mxesai_capture_release (HW_CONTEXT_T * mx, PCM_SUBCHN_CONTEXT_T * strm,
	ado_pcm_config_t * config)
{
	ado_mutex_lock (&mx->hw_lock);
	ado_pcm_buf_free(config);
	strm->pcm_subchn = NULL;
	strm->pcm_config = NULL;
	ado_mutex_unlock (&mx->hw_lock);
	return (0);
}

int32_t
mxesai_prepare (HW_CONTEXT_T * mx, PCM_SUBCHN_CONTEXT_T * strm, ado_pcm_config_t * config)
{
	strm->pcm_offset = 0;
	return (0);
}

/*
 * This function is used when more than 1 playback pcm device is created. It is called from the
 * tx interrupt handler (or pulse) and trigger function to combine multiple pcm streams into 1
 * hardware TDM stream.
 */
static void
mxesai_dmaplayback_combine(HW_CONTEXT_T *mx, uint8_t *dstDMAAddr, uint32_t size, bool calldmainterrupt)
{
	int32_t sample_offset = 0, cnt = 0, voice = 0, slot_cnt;
	uint16_t *buf;
	uint16_t *dmaptr = (uint16_t*) dstDMAAddr;

	if((NULL == dstDMAAddr) || (0 == size))
	{
		ado_error("%s: invalid data (0x%x) or size (%d)", __FUNCTION__, dstDMAAddr, size);
		return;
	}

	while ((sample_offset * SAMPLE_SIZE) < size)
	{
		for (cnt = 0, slot_cnt = 0; cnt < mx->num_tx_aif; cnt++ )
		{
			if (mx->aif[cnt].play_strm.active)
			{
				if ((mx->aif[cnt].play_strm.pcm_offset * SAMPLE_SIZE ) >= mx->aif[cnt].play_strm.pcm_config->dmabuf.size)
					mx->aif[cnt].play_strm.pcm_offset = 0;
				buf = (uint16_t*) mx->aif[cnt].play_strm.pcm_config->dmabuf.addr + mx->aif[cnt].play_strm.pcm_offset;

				for (voice = 0; voice < mx->aif[cnt].play_strm.voices; voice++)
					dmaptr[sample_offset + voice] = buf[voice];
				/* Bump the pcm buffer offset */
				mx->aif[cnt].play_strm.pcm_offset += mx->aif[cnt].play_strm.voices;
				if (((mx->aif[cnt].play_strm.pcm_offset * SAMPLE_SIZE) % ado_pcm_dma_int_size(mx->aif[cnt].play_strm.pcm_config)) == 0)
				{
					// Signal to io-audio (DMA transfer was completed)
					if (calldmainterrupt)
						dma_interrupt(mx->aif[cnt].play_strm.pcm_subchn);
				}
			}
			else
			{
				/* Silence fill the inactive TDM slots */
				for (voice = 0; voice <  mx->aif[cnt].play_strm.voices; voice++)
					dmaptr[sample_offset + voice] = 0x0000;
			}
			slot_cnt += mx->aif[cnt].play_strm.voices;
			sample_offset += mx->aif[cnt].play_strm.voices;
		}
		/* Pad the remaining slots/words if the combined number of voices/channels accross
		 * all of our playback interfaces is less then configured nslots
		 */
		for (;slot_cnt < mx->nslots; slot_cnt++)
		{
			dmaptr[sample_offset++] = 0x0000;
		}
	}
	/* If multiple pins are used call reconstitute to interleave the data over the pins */
	if (mx->num_sdo_pins > 1)
		mxesai_playback_reconstitute (mx, NULL, (int8_t *) dmaptr, size);

	/* Flush cache */
	msync(dstDMAAddr, size, MS_SYNC);

	if (mx->log_enabled)
		write(mx->play_log_fd, dmaptr, size);
}

int open_log (HW_CONTEXT_T * mx, int chn_type)
{
	time_t t = time(NULL);
	struct tm *current_time = gmtime ( &t );
	char filename_buf[_POSIX_PATH_MAX];
	
	/* RAW PCM file, no wave header */
	if (chn_type == SND_PCM_CHANNEL_PLAYBACK)
	{
		snprintf(filename_buf, _POSIX_PATH_MAX, "/dev/shmem/playback_raw_%04d%02d%02d-%02d%02d%02dUTC.wav",
			1900 + current_time->tm_year, current_time->tm_mon + 1, current_time->tm_mday, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
	
		if ((mx->play_log_fd = open ( filename_buf, O_WRONLY | O_CREAT, 0400)) == -1)
		{
			ado_error("Failed to create PCM log file (%s) - %s", filename_buf, strerror(errno));
			return (-errno);
		}
	}
	else
	{
		snprintf(filename_buf, _POSIX_PATH_MAX, "/dev/shmem/capture_raw_%04d%02d%02d-%02d%02d%02dUTC.wav",
			1900 + current_time->tm_year, current_time->tm_mon + 1, current_time->tm_mday, current_time->tm_hour, current_time->tm_min, current_time->tm_sec);
	
		if ((mx->cap_log_fd = open ( filename_buf, O_WRONLY | O_CREAT, 0400)) == -1)
		{
			ado_error("Failed to create PCM log file (%s) - %s", filename_buf, strerror(errno));
			return (-errno);
		}
	}
	return EOK;
}

int close_log ( HW_CONTEXT_T *mx, int chn_type)
{
	if (chn_type == SND_PCM_CHANNEL_PLAYBACK)
	{
		fsync( mx->play_log_fd );
		close(mx->play_log_fd);
	}
	else
	{
		fsync( mx->cap_log_fd );
		close(mx->cap_log_fd);
	}

	return EOK;
}

int32_t
mxesai_playback_trigger (HW_CONTEXT_T * mx, PCM_SUBCHN_CONTEXT_T * strm, uint32_t cmd)
{
	int rtn = EOK, i = 0, pin;
	ado_mutex_lock (&mx->hw_lock);

	if (cmd == ADO_PCM_TRIGGER_GO)
	{
		if (num_active_playback_interfaces(mx) == 0)
		{
			if (mx->log_enabled)
				open_log(mx, SND_PCM_CHANNEL_PLAYBACK);
			strm->active = 1;
			mx->playback_frag_index = 0;
			if (mx->num_tx_aif > 1)
			{
				/* Combine PCM data from the various PCM playback devices into the DMA buffer,
				 * Fill the entire DMA buffer (both fragments), we will backfill the DMA buffer
				 * on the interrupt completion interrupt/event
				 */
				/* We cannot call dma_interrupt more than once from the trigger since the sw_mix processing
				 * cannot complete until trigger function completes.
				*/
				mxesai_dmaplayback_combine(mx, (uint8_t*)(&mx->playback_dmabuf.addr[0]), (mx->playback_dmabuf.size / 2), false);
				mxesai_dmaplayback_combine(mx, (uint8_t*)(&mx->playback_dmabuf.addr[mx->playback_dmabuf.size / 2]), (mx->playback_dmabuf.size / 2), true);
			}

			/* Take TX FIFO out of reset */
			mx->esai->tfcr &= ~(TFCR_TFR);

			/* Enable requested transmitters to use FIFO (must be done before FIFO itself is enabled) */
			for (i = 0; i < MAX_NUM_SDO_PINS; i++)
			{
				pin = 0x1 << i;
				if (mx->sdo_pins & pin)
				{
					mx->esai->tfcr |= TFCR_TE(i);
				}
			}

			/* MSB of data is bit 23; TX watermark is 32, init TX register from FIFO */
			mx->esai->tfcr |= TFCR_TFWM(32) | TFCR_TWA(2) | TFCR_TIEN;

			/* Enable TX FIFO */
			mx->esai->tfcr |= TFCR_TFE;

			if (mx->sdmafuncs.xfer_start (mx->aif[0].play_strm.dma_chn) == -1)
			{
				rtn = errno;
				ado_error ("MX ESAI: Audio DMA Start failed (%s)", strerror(rtn));
			}

			/* Give DMA time to fill the FIFO */
			while (((mx->esai->tfsr&0xff) < 16) && (i<MAX_WAITTIME_US))
			{
				i++;
				nanospin_ns(1000);
			}

			/* Report an error if DMA didn't fill the FIFO on time, but still keep running */
			if (i>=MAX_WAITTIME_US) 
			{
				rtn = ETIME;
				ado_error ("MX ESAI: Audio TX FIFO underrun");
			}

			/* Enable TX (also configures the SD pin to function as an output) */
			for (i = 0; i < MAX_NUM_SDO_PINS; i++)
			{
				pin = 0x1 << i;
				if (mx->sdo_pins & pin)
				{
					mx->esai->tcr |= TCR_TE(i);
				}
			}
			ado_debug (DB_LVL_DRIVER, "%s: TFCR=0x%x, TCR=0x%x", __FUNCTION__, mx->esai->tfcr, mx->esai->tcr);
		}
		else
			strm->active = 1;
	}
	else
	{
		if (num_active_playback_interfaces(mx) == 1)
		{
			/* Abort DMA transfer */
			if (mx->sdmafuncs.xfer_abort (mx->aif[0].play_strm.dma_chn) == -1)
			{
				rtn = errno;
				ado_error ("MX ESAI: Audio DMA Stop failed (%s)", strerror(rtn));
			}

			/* Wait for FIFO to drain 
			 * Wait up to 16ms for FIFO (256bytes) to drain
			 * 16 = 256 / (8khz * 2bytes) * 1000
			 */
			while (((mx->esai->tfsr&0xff) > 0) && (i<16))
			{
				i++;
				delay(1);
			}

			/* Wait for the transmit shift register to empty (underrun) */
			i = 1000;
			while(!(mx->esai->saisr & (SAISR_TDE | SAISR_TUE)) && i-- > 0)
				nanospin_ns(100);
			if (i <= 0)
				ado_error("%s: TXSR failed to empty, saisr = 0x%x", __FUNCTION__, mx->esai->saisr);

			/* Disable FIFO and transmitters
			 * NOTE: Must disable FIFO before disabling transmitters
			 */
			mx->esai->tfcr &= ~TFCR_TFE;
			mx->esai->tfcr &= ~TFCR_TEMASK;
			mx->esai->tcr &= ~TCR_TEMASK;

			if (mx->log_enabled)
				close_log(mx, SND_PCM_CHANNEL_PLAYBACK);

			/* Put TX FIFO into reset */
			mx->esai->tfcr |= TFCR_TFR;
		}
		strm->active = 0;
	}

	ado_mutex_unlock (&mx->hw_lock);
	return (rtn);
}

int32_t
mxesai_capture_trigger (HW_CONTEXT_T * mx, PCM_SUBCHN_CONTEXT_T * strm, uint32_t cmd)
{
	int rtn = EOK, i, pin;

	ado_mutex_lock (&mx->hw_lock);

	if (cmd == ADO_PCM_TRIGGER_GO)
	{
		if (num_active_capture_interfaces(mx) == 0)
		{
			if (mx->log_enabled)
				open_log(mx, SND_PCM_CHANNEL_CAPTURE);

			/* Take RX FIFO out of reset */
			mx->esai->rfcr &= ~(RFCR_RFR);
			/* watermark 32, MSB of data is bit 23 */
			mx->esai->rfcr = RFCR_RFWM(32) | RFCR_RWA(2);

			/* Enable requested receivers to use FIFO (must be done before FIFO itself is enabled) */
			for (i = 0; i < MAX_NUM_SDI_PINS; i++)
			{
				pin = 0x1 << i;
				if (mx->sdi_pins & pin)
				{
					mx->esai->rfcr |= RFCR_RE(i);
				}
			}

			/* Enable RX FIFO */
			mx->esai->rfcr |= RFCR_RFE;
			/* Take RX out of personal reset */
			mx->esai->rcr &= ~(RCR_RPR);

			mx->capture_frag_index = 0;
			if (mx->sdmafuncs.xfer_start (mx->aif[0].cap_strm.dma_chn) == -1)
			{
				rtn = errno;
				ado_error ("MX ESAI: Audio DMA Start failed (%s)", strerror(rtn));
			}

			/* Enable RX (also configures the SD pin to function as an input) */
			for (i = 0; i < MAX_NUM_SDI_PINS; i++)
			{
				pin = 0x1 << i;
				if (mx->sdi_pins & pin)
				{
					mx->esai->rcr |= RCR_RE(i);
				}
			}
			ado_debug (DB_LVL_DRIVER, "%s: RFCR=0x%x, RCR=0x%x", __FUNCTION__, mx->esai->rfcr, mx->esai->rcr);
		}
		strm->active = 1;
	}
	else
	{
		if (num_active_capture_interfaces(mx) == 1)
		{
			/* Disable FIFO and receivers
			 * NOTE: Must disable FIFO before disabling receivers
			 */
			mx->esai->rfcr &= ~RFCR_RFE;
			mx->esai->rfcr &= ~RFCR_REMASK;
			mx->esai->rcr &= ~RCR_REMASK;

			if (mx->sdmafuncs.xfer_abort (mx->aif[0].cap_strm.dma_chn) == -1)
			{
				rtn = errno;
				ado_error ("MX ESAI: Audio DMA Stop failed (%s)", strerror(rtn));
			}
			if (mx->log_enabled)
				close_log(mx, SND_PCM_CHANNEL_CAPTURE);

			/* Put RX FIFO into reset and the receiver into personal reset (does not affect control bits) */
			mx->esai->rfcr |= RFCR_RFR;
			mx->esai->rcr |= RCR_RPR;
		}
		strm->active = 0;
	}

	ado_mutex_unlock (&mx->hw_lock);
	
	return (rtn);
}

/*
 * No position function as we are unable to get the transfer count of the
 * current DMA operation from the SDMA microcode. The resolution of the
 * positional information returned to the client will be limited to the
 * fragment size.
 *
 * If we get new SDMA microcode that supports this and the dma library's
 * bytes_left() function is updated to use this info to return the actual
 * bytes left, uncomment the below function and the function pointer
 * aesaignments in ctrl_init().
 */
 
#if 0
uint32_t
mxesai_position (HW_CONTEXT_T * mx, PCM_SUBCHN_CONTEXT_T * pc, ado_pcm_config_t * config)
{
	uint32_t pos;

	ado_mutex_lock (&mx->hw_lock);

	if (pc == mx->play_strm.pcm_subchn)
	{
		pos =
			ado_pcm_dma_int_size (config) -
			mx->sdmafuncs.bytes_left (mx->play_strm.dma_chn);
	}
	else
	{
		pos =
			ado_pcm_dma_int_size (config) -
			mx->sdmafuncs.bytes_left (mx->cap_strm.dma_chn);
	}

	ado_mutex_unlock (&mx->hw_lock);

	return (pos);
}
#endif

/**
 * This function is used when more than 1 capture pcm devices created. It is called from
 * rx interrupt handler (or pulse) to split the hardware stream into multiple pcm streams
 */
static void
mxesai_dmacapture_split(HW_CONTEXT_T *mx, uint8_t *srcDMAAddr, uint32_t size)
{
	int32_t sample_offset = 0, cnt = 0, voice = 0, frame_offset = 0;
	uint16_t *buf;
	uint16_t *dmaptr = (uint16_t*) srcDMAAddr;
	int16_t frame_size = mx->nslots * mx->num_sdi_pins;

	if((NULL == srcDMAAddr) || (0 == size))
	{
		ado_error("%s: invalid data (0x%x) or size (%d)", __FUNCTION__, srcDMAAddr, size);
		return;
	}

	/* Invalidate cache */
	msync(srcDMAAddr, size, MS_INVALIDATE);

	if (mx->log_enabled)
		write(mx->cap_log_fd, dmaptr, size);

	while ((sample_offset * SAMPLE_SIZE) < size)
	{
		for (cnt = 0, frame_offset = 0; cnt < mx->num_rx_aif; cnt++ )
		{
			if (mx->aif[cnt].cap_strm.active)
			{
				if ((mx->aif[cnt].cap_strm.pcm_offset * SAMPLE_SIZE ) >= mx->aif[cnt].cap_strm.pcm_config->dmabuf.size)
					mx->aif[cnt].cap_strm.pcm_offset = 0;
				buf = (uint16_t*) mx->aif[cnt].cap_strm.pcm_config->dmabuf.addr + mx->aif[cnt].cap_strm.pcm_offset;

				for (voice = 0; voice < mx->aif[cnt].cap_strm.voices; voice++)
					buf[voice] = dmaptr[sample_offset + mx->sdi_chmap[frame_offset + voice]];

				/* Bump the pcm buffer offset */
				mx->aif[cnt].cap_strm.pcm_offset += mx->aif[cnt].cap_strm.voices;
				if (((mx->aif[cnt].cap_strm.pcm_offset * SAMPLE_SIZE) % ado_pcm_dma_int_size(mx->aif[cnt].cap_strm.pcm_config)) == 0)
				{
					/* Signal to io-audio (DMA transfer was completed) */
					dma_interrupt(mx->aif[cnt].cap_strm.pcm_subchn);
				}
			}
			/* Bump the frame offset */
			frame_offset += mx->aif[cnt].cap_strm.voices;
		}
		sample_offset += frame_size;
	}
}

int32_t
mxesai_playback_reconstitute (HW_CONTEXT_T *mx, PCM_SUBCHN_CONTEXT_T *pc, int8_t *dmaptr, size_t size)
{
    int16_t i, cnt, *ptr;
    int frame_size = mx->nslots * mx->num_sdo_pins;

    ptr = (int16_t *) dmaptr;

    /* When using multiple data output lines,
     * each sample shifted out will go to the next active output line,
     * so we must interleave the data over the active pins
     * (i.e. Left and right samples on the same output rather than split over multiple outputs).
     */
    for (i = 0; i < (size / sizeof (int16_t)); i+=frame_size)
    {
        for (cnt = 0; cnt < frame_size; cnt++)
            mx->sdo_reconstitute_buffer[cnt] = ptr[i + mx->sdo_chmap[cnt]];
        memcpy (&ptr[i], mx->sdo_reconstitute_buffer, frame_size*sizeof(int16_t));
    }
    return (0);
}

int32_t
mxesai_capture_reconstitute (HW_CONTEXT_T *mx, PCM_SUBCHN_CONTEXT_T *pc, int8_t *dmaptr, size_t size)
{
    int16_t i, cnt, *ptr;
    int frame_size = mx->nslots * mx->num_sdi_pins;

    ptr = (int16_t *) dmaptr;

    /* When using multiple data input lines,
     * each sample shifted in will be from the next active input line,
     * so we must de-interleave the data shifted in from the the active pins
     */
    for (i = 0; i < (size / sizeof (int16_t)); i+=frame_size)
    {
        for (cnt = 0; cnt < frame_size; cnt++)
            mx->sdi_reconstitute_buffer[cnt] = ptr[i + mx->sdi_chmap[cnt]];
        memcpy (&ptr[i], mx->sdi_reconstitute_buffer, frame_size*sizeof(int16_t));
    }
    return (0);
}

void
mxesai_play_pulse_hdlr (HW_CONTEXT_T * mx, struct sigevent *event)
{
	if (mx->num_tx_aif == 1)
	{
		if (mx->aif[0].play_strm.pcm_subchn)
		{
			if (mx->log_enabled)
			{
				uint8_t *dmaptr;
				ado_pcm_config_t *config = mx->aif[0].play_strm.pcm_config;

				if (mx->playback_frag_index >= config->mode.block.frags_total) 
					mx->playback_frag_index = 0;

				dmaptr = (uint8_t*)(&config->dmabuf.addr[mx->playback_frag_index * ado_pcm_dma_int_size(config)]);
				write(mx->play_log_fd, dmaptr, ado_pcm_dma_int_size(config));
				mx->playback_frag_index++;
			}
			dma_interrupt (mx->aif[0].play_strm.pcm_subchn);
		}
	}
	else
	{
		if (mx->playback_frag_index >= 2)
			mx->playback_frag_index = 0;
		/* We use a ping-ping buffer for DMA, so divide buffer size by 2 to get the frag size */
		mxesai_dmaplayback_combine(mx, (uint8_t*)(&mx->playback_dmabuf.addr[mx->playback_frag_index * (mx->playback_dmabuf.size / 2)]), (mx->playback_dmabuf.size / 2), true);
		mx->playback_frag_index++;
	}
}

void
mxesai_cap_pulse_hdlr (HW_CONTEXT_T * mx, struct sigevent *event)
{
	if ( mx->num_rx_aif == 1)
	{
		if (mx->aif[0].cap_strm.pcm_subchn)
		{
			if (mx->log_enabled)
			{
				uint8_t *dmaptr;
				ado_pcm_config_t *config = mx->aif[0].cap_strm.pcm_config;

				if (mx->capture_frag_index >= config->mode.block.frags_total) 
					mx->capture_frag_index = 0;

				dmaptr = (uint8_t*)(&config->dmabuf.addr[mx->capture_frag_index * ado_pcm_dma_int_size(config)]);
				write(mx->cap_log_fd, dmaptr, ado_pcm_dma_int_size(config));
				mx->capture_frag_index++;
			}
			dma_interrupt (mx->aif[0].cap_strm.pcm_subchn);
		}
	}
	else
	{
		if (mx->capture_frag_index >= 2) 
			mx->capture_frag_index = 0;
		/* We use a ping-ping buffer from DMA, so divide buffer size by 2 to get the frag size */
	 	mxesai_dmacapture_split(mx, (uint8_t*)(&mx->capture_dmabuf.addr[mx->capture_frag_index * (mx->capture_dmabuf.size / 2)]), (mx->capture_dmabuf.size / 2));
		mx->capture_frag_index++;
	}
}

int
mxesai_init (HW_CONTEXT_T * mx)
{
	/* person_reset_val contains a bitfield which specifies the direction
	 * of the ESAI pins and whether they are connected or not */
	uint32_t personal_reset_val = 0xfff;
	uint32_t slot_bits, slot_mask = 0;
	uint32_t voices, cnt;

	/* Enable ESAI */
	mx->esai->ecr = ECR_ESAIEN;	

	/* Reset ESAI */
	mx->esai->ecr = ECR_ESAIEN | ECR_ERST;
	delay (1);

	/* Clear Reset */
	mx->esai->ecr = ECR_ESAIEN;

	/* Disconnect ESAI port */
	mx->esai->pcrc = 0;	
	mx->esai->prrc = 0;

	mx->esai->tccr = mx->esai->rccr = 0;

	/* Synchronous mode (RX shares clocks with TX), left align to 16 bit */
	mx->esai->saicr = SAICR_SYNC | SAICR_ALC;

	/* Synchronous mode, therefore tx clocks shared with rx clocks */
	personal_reset_val &= ~(PORT0_SCKR | PORT1_FSR | PORT2_HCKR);

	if (mx->bit_delay)
	{
		/* 1-bit data delay */
		mx->esai->tcr |= TCR_TFSR; 
		mx->esai->rcr |= RCR_RFSR;
	}

	if (mx->fs_active_width == FS_BIT)
	{
		mx->esai->tcr |= TCR_TFSL;
		mx->esai->rcr |= RCR_RFSL;
	}

	switch (mx->slot_size)
	{
		case 16:
			slot_bits = 0x2;
			break;
		default:
		case 32:
			slot_bits = 0x12;
			break;
	}

	/* 
	 * word length frame sync; 16 bit word length; slot size 16 or 32 bits,
	 * network mode, Transmit zero padding
	 */
	mx->esai->tcr |= TCR_TSWS(slot_bits) | TCR_TMOD (1) | TCR_PADC; 
	mx->esai->rcr |= RCR_RSWS(slot_bits) | RCR_RMOD (1);

	/* RX Slot mask */
	for (cnt = 0, voices = 0; cnt < mx->num_rx_aif; cnt++)
		voices += mx->aif[cnt].cap_strm.voices;
	for (cnt = 0; cnt < voices; cnt++)
		slot_mask |= (1 << cnt);
	mx->esai->rsma = (slot_mask & 0xffff);
	mx->esai->rsmb = (slot_mask >> 16);

#if defined(VARIANT_mx6q_ddr3)
	char mixer_params[100];
	/*
	 * Inform mixer that the SOC is the master. Note that the i.MX6 Q DDR3 evaluation board
	 * requires that the ESAI module be the clock master in order for the codec to work correctly.
	 */
	sprintf(mixer_params, ":mode=master");
	strcat(mx->mixeropts, mixer_params);

	/* Inform mixer that the codec is using Left Justified format */
	sprintf(mixer_params, ":format=1");
	strcat(mx->mixeropts, mixer_params);
#endif

	mxesai_set_clock_rate(mx,  mx->sample_rate_max);

	ado_debug(DB_LVL_DRIVER, "%s: mclk = %d, sample rate = %d, nslots = %d, slot size = %d", __FUNCTION__, mx->main_clk_freq, mx->sample_rate, mx->nslots, mx->slot_size);

	/* Set which ports connect to ESAI*/
	mx->esai->prrc = personal_reset_val;
	mx->esai->pcrc = personal_reset_val;

	/*  Put TX FIFO into reset
	 *  NOTE: Because the RX runs off the TX clock (Synchronous clocks) we cannot put the transmitter into personal reset
	 */
	mx->esai->tfcr = TFCR_TFR;

	/* Put the RX FIFO into reset and the receiver into personal reset (does not affect control bits) */
	mx->esai->rfcr = RFCR_RFR;
	mx->esai->rcr |= RCR_RPR;

	return 0;
}

int 
mxesai_set_clock_rate ( HW_CONTEXT_T *mx, int rate)
{
	uint32_t bit_clk_div, tccr, rccr;

	tccr = rccr = 0;
	mx->sample_rate = rate;

	if (mx->fsync_pol == 0)
	{
		/* Active low Frame sync */
		tccr |= TCCR_TFSP;
		rccr |= RCCR_RFSP;
	}

	if (mx->xclk_pol == 0)
		tccr |= TCCR_TCKP;	/* TX on falling edge */

	if (mx->rclk_pol == 0)
		rccr |= RCCR_RCKP;	/* RX on falling edge */

	/* Slots/words per frame (program 1 less then desired value as per docs) */
	tccr |= TCCR_TDC((mx->nslots - 1));
	rccr |= RCCR_RDC((mx->nslots - 1));

	/* Calculate MCLK divisor.
	 * NOTE: The current implementation supports synchronous mode only -
	 *       (SAICR[SYN]=1) where the receiver uses the tx clock control register
	 *       settings. However, we still program the RCCR in the event that we
	 *       need to support asynchronous clocking in the future.
	 */
#define FIXED_DIV2 2
	bit_clk_div = mx->main_clk_freq / (FIXED_DIV2 * mx->sample_rate * mx->nslots * mx->slot_size);

	if (bit_clk_div <= 256)
	{
		// Bypass Div8 (i.e. TPSR/RPSP=1 which results in divide by 1), use TPM/RPM divider, don't use TFP/RFP divider
		tccr |= TCCR_TPSR | TCCR_TPM((bit_clk_div-1)) | TCCR_TFP(0);
		rccr |= RCCR_RPSP | RCCR_RPM((bit_clk_div-1)) | RCCR_RFP(0);
	}
	else
	{
		if (bit_clk_div <= (256*8))
		{
			// Don't bypass Div8 (TPSR=0), use TPM/RPM divider, don't use TFP/RFP divider
			tccr |= TCCR_TPM(((bit_clk_div/8)-1)) | TCCR_TFP(0);
			rccr |= RCCR_RPM(((bit_clk_div/8)-1)) | RCCR_RFP(0);
		}
		else
		{
			// Don't bypass Div8 (TPSR=0), use TPM/RPM divider, use TFP/RFP divider
			tccr |= TCCR_TPM(256) | TCCR_TFP(((bit_clk_div/(8*256))-1));
			rccr |= RCCR_RPM(256) | RCCR_RFP(((bit_clk_div/(8*256))-1));
		}
	}

	switch (mx->main_clk_src)
	{
		/*
		 * 'XTAL' refers to an SOC generated clock.  The clock to supply, and divisors are
		 * are selected by the CCM Serial Clock Multiplexor Register, and Clock Divider Register.
		 */
		case MAIN_CLK_SRC_XTAL:

			/* word length frame sync; slot length = 16, word length = 16; network mode */
			mx->esai->tcr |= TCR_TSWS(0x02) | TCR_TMOD (1);

			if (mx->clk_mode == ESAI_CLK_MASTER)
			{
				/*
				 * THCKD=1: High frequency clock is an output
				 * TCKD=1:  SCKT (bit clock) output
				 * TFSD=1:  FST (frame sync) output
				 */
				tccr |= TCCR_THCKD | TCCR_TCKD | TCCR_TFSD;

				/*
				 * RHCKD=1: High Frequency clock direction output (not needed...)
				 * RCKD=1:  Receiver bit clock is an output
				 * RFSD=1:  Receiver frame sync is an output
				 */
				rccr |= RCCR_RHCKD | RCCR_RCKD | RCCR_RFSD;
			}

			/* HCKT clock will be derived from XTAL.  The on-chip esai clock source is
			 * specified by the CCM, the CCM refers to this clock as 'esai_clk_root'.
			 */
			mx->esai->ecr |= ECR_ETI | ECR_ERI;

			/* Prevent XTAL clock override */
			mx->esai->ecr &= ~(ECR_ETO | ECR_ERO);

			break;

		/* 'FSYS' refers to the SOC generated ESAI system clock, which is typically 133MHz */
		case MAIN_CLK_SRC_FSYS:
			ado_error("%s: Clock source must be either XTAL or OSC", __FUNCTION__);
			return -1;

		/* 'OSC' refers to an external Oscillator */
		case MAIN_CLK_SRC_OSC:
			if (mx->clk_mode == ESAI_CLK_MASTER)
			{
				/*
				 * THCKD = 0: High frequency clock is an input
				 * TCKD = 1: SCKT is an output (use internal bit and frame sync generators)
				 * TFSD = 1: FST (frame sync) is an output
				 *
				 * NOTE: When the internal ARM Core is the clock source you cannot
				 *       set TPSR (divide by 8 bypass), and TPM[7-0] = 0x0 and TFP[3-0] = 0x0
				 *       as this will cause clock synchronization problems...in this case
				 *       the HCKT is the clock source so we should be okay.
				 */
				tccr |= TCCR_TCKD | TCCR_TFSD;

				/*
				 * RHCKD = 0: High frequency clock is an input
				 * RCKD = 1: SCKT is an output (use internal bit and frame sync generators)
				 * RFSD = 1: FST (frame sync) is an output
				 */
				rccr |= RCCR_RCKD | RCCR_RFSD;
			}

			break;

		default:
			ado_error ("%s: main_clk_src value not supported", __FUNCTION__);
			return -1;

	}

	ado_debug(DB_LVL_DRIVER, "%s: tccr=0x%x, rccr=0x%x", __FUNCTION__, tccr, rccr);

	// only program tccr and rccr registers once after all bits have been set, otherwise
	// bit clock and frame clock will not be set correctly.
	mx->esai->tccr = tccr;
	mx->esai->rccr = rccr;
	usleep(125);

	return (EOK);
}
ado_dll_version_t ctrl_version;
void
ctrl_version (int *major, int *minor, char *date)
{
	*major = ADO_MAJOR_VERSION;
	*minor = 1;
	date = __DATE__;
}

int *
set_reconstitute_map(HW_CONTEXT_T *mx, int chn, char *reconstitute_map)
{

	int pins = (chn == SND_PCM_CHANNEL_PLAYBACK) ? mx->num_sdo_pins : mx->num_sdi_pins;
	int frame_size = mx->nslots * pins;
	int *map = NULL;
	int idx;
	const char *chn_name = (chn == SND_PCM_CHANNEL_PLAYBACK) ? "playback" : "capture";

	/* Allocate a reconstitute channel map */
	if ((map = ado_calloc (frame_size, sizeof(int))) == NULL)
	{
		ado_error ("MX ESAI: Unable to allocate %s reconstitute channel map (%s)", chn_name, strerror (errno));
		return (NULL);
	}

	if (reconstitute_map == NULL)
	{
		int pin, slot, frame_offset = 0;

		ado_debug(DB_LVL_DRIVER, "%s: Using default %s reconstitute mapping", __FUNCTION__, chn_name);
		if (chn == SND_PCM_CHANNEL_PLAYBACK)
		{
			/* For Playback the data comes from the combine function with the channels/slots per
			 * sdo line contiguious in memory. The ESAI will shift out one channel/slot per sdo pin,
			 * so we must interleave the channels/slots in memory so that they appear contiguous on the
			 * sdo pins
			 */
			for (pin = 0; pin < pins; pin++)
			{
				for (slot = 0; slot < mx->nslots; slot++)
					map[(slot * pins) + pin] = frame_offset++;
			}
		}
		if (chn == SND_PCM_CHANNEL_CAPTURE)
		{
			/* For Capture the data comes from the MXESAI with the channels/slots interleaved,
			 * (one channel/slot per sdi pin shifted into memroy). We must de-interleave
			 * the channels/slots so that the channels/slots per sdi pin are contiguous
			 * in memory for the splitter function to break apart into separate device
			 * buffers
			 */
			for (pin = 0; pin < pins; pin++)
			{
				for (slot = 0; slot < mx->nslots; slot++)
					map[frame_offset++] = (slot * pins) + pin;
			}
		}
	}
	else
	{
		char *value = reconstitute_map, *value2;

		map[0] = strtoul(value, 0, 0);
		if (map[0] >= frame_size)
		{
			ado_debug(DB_LVL_DRIVER, "mxesai: Invalid %s chmap[%d] channels %d", chn_name, 0, map[0]);
			ado_free(map);
			return (NULL);
		}

		idx = 1;	/* Skip first character since we handled it above */
		while (idx < frame_size && (value2 = strchr(value, ':')) != NULL)
		{
			map[idx++] = strtoul(value2 + 1, 0, 0);
			if (map[idx - 1] >= frame_size)
			{
				ado_debug(DB_LVL_DRIVER, "mxesai: Invalid %s champ[%d] %d", chn_name, idx - 1, map[idx - 1]);
				ado_free(map);
				return (NULL);
			}
			value = value2+1;
		}
	}

	if (global_options.db_lvl & DB_LVL_DRIVER)
	{
		for(idx = 0; idx < frame_size; idx++)
			ado_error("%s chmap[%d] = %d", chn_name, idx, map[idx]);
	}

	return (map);
}

/*
esaibase = [#]; base address of esai controller
tevt    = [#]; esai TX DMA event number
tchn    = [#]; esai TX DMA channel type 
revt    = [#]; esai RX DMA event number
rchn    = [#]; esai RX DMA channel type
rate    = [#]; sample rate of audio
mixer = [[mixer option1]:[mixer options2][:[other options]]:[info]]
   mixer=info to dump the details of mixer options
*/

static char *
mxesai_opts[] = {
#define OPT_ESAIBASE      0
	"esaibase",
#define OPT_TEVT          1
	"tevt",
#define OPT_TCHN          2
	"tchn",
#define OPT_REVT          3
	"revt",
#define OPT_RCHN          4
	"rchn",
#define OPT_RATE          5
	"rate",
#define OPT_MAIN_CLK_SRC  6
	"main_clk_src",
#define OPT_MAIN_CLK_FREQ 7
	"main_clk_freq",
#define OPT_MIXER         8
	"mixer",
#define OPT_RX_VOICES     9
	"rx_voices",
#define OPT_I2C_BUS       10
	"i2c_bus",
#define OPT_I2C_ADDR      11
	"i2c_addr",
#define OPT_SDO_PINS      12
	"sdo_pin_map",
#define OPT_SDI_PINS      13
	"sdi_pin_map",
#define OPT_CLK_MODE      14
	"clk_mode",
#define OPT_NSLOTS        15
	"nslots",
#define OPT_DEBUG         16
	"debug",
#define OPT_RX_FRAG_SIZE  17
	"rx_frag_size",
#define OPT_TX_FRAG_SIZE  18
	"tx_frag_size",
#define OPT_PROTOCOL      19
	"protocol",
#define OPT_BIT_DELAY     20
	"bit_delay",
#define OPT_FSYNC_POL     21
	"fsync_pol",
#define OPT_SLOT_SIZE     22
	"slot_size",
#define OPT_TX_VOICES     23
	"tx_voices",
#define OPT_XCLK_POL      24
	"xclk_pol",
#define OPT_RCLK_POL      25
	"rclk_pol",
#define OPT_TX_CHMAP      26
	"sdo_chmap",
#define OPT_RX_CHMAP      27
	"sdi_chmap",
#define OPT_FS_ACTIVE_WIDTH  28
	"fs_active_width",
	NULL
};

static void 
build_dma_string (char * dmastring, uint32_t fifopaddr, int dmaevent, int watermark)
{
	char str[50];
	
	strcpy (dmastring, "eventnum=");
	strcat (dmastring, itoa (dmaevent, str, 10));
	strcat (dmastring, ",watermark=");
	strcat (dmastring, itoa (watermark, str, 10));
	strcat (dmastring, ",fifopaddr=0x");
	strcat (dmastring, ultoa (fifopaddr, str, 16));
	strcat (dmastring, ",regen,contloop");
		
}

/* This function configures the various protocol specific flags
 */
void configure_default_protocol_flags(HW_CONTEXT_T * mx)
{
	switch (mx->protocol)
	{
		case PROTOCOL_PCM:
			mx->bit_delay = 0;      /* 0-bit clock delay       */
			mx->fsync_pol = 1;      /* Active high frame sync  */
			mx->xclk_pol = 1;       /* TX on BCLK rising edge  */
			mx->rclk_pol = 1;       /* TX on BCLK rising edge  */
			break;
		case PROTOCOL_I2S:
			mx->bit_delay = 1;		/* 1-bit clock delay       */
			mx->fsync_pol = 0;		/* Active low frame sync   */
			mx->xclk_pol = 0;       /* TX on BCLK falling edge */
			mx->rclk_pol = 1;       /* RX on BCLK rising edge  */
			break;
		default:
			break;
	}
}

/* Convert string list of pins into a bit map */
static int16_t parse_pin_list(const char* pin_list, int16_t* pin_bit_map)
{
	int idx;
	int cnt;
	char *next;
	int pin_number;

	/* Count number of pins in the pin_list */
	for (idx = 0, cnt = 1; idx < strlen(pin_list); (pin_list[idx] == ':') ? cnt++ : 0, idx++);

	/* Convert first pin into a bit map */
	pin_number = strtoul(pin_list, 0, 0);
	*pin_bit_map = 0x1 << pin_number;

	idx = 1;	/* Skip first pin since we handled it above */
	/* Convert the remaining list of pins (if any) into a bit map */
	while (idx < cnt && (next = strchr(pin_list, ':')) != NULL)
	{
		pin_number = strtoul(next + 1, 0, 0);
		*pin_bit_map |= 0x1 <<  pin_number;
		pin_list = next+1;
	}

	return cnt;
}

int mxesai_parse_commandline(HW_CONTEXT_T * mx, char *args)
{
	char     *value;
	char     *tx_voices = NULL;
	char     *rx_voices = NULL;
	char     *sdo_chmap = NULL;
	char     *sdi_chmap = NULL;
	char     *sdo_pin_map = NULL;
	char     *sdi_pin_map = NULL;
#if defined(ESAI_BASE_ADDR)
	mx->esaibase = ESAI_BASE_ADDR;
#else
	/* Getting the ESAI Base addresss from the Hwinfo Section if available */
	unsigned hwi_off = hwi_find_device ("esai", 0);
	if(hwi_off != HWI_NULL_OFF)
	{
		hwi_tag *tag = hwi_tag_find (hwi_off, HWI_TAG_NAME_location, 0);
		if(tag)
			mx->esaibase = tag->location.base;
	}
#endif
	strcpy (mx->mixeropts, "");
	mx->tevt = ESAI_TX_DMA_EVENT;
	mx->tchn = ESAI_TX_DMA_CTYPE;
	mx->revt = ESAI_RX_DMA_EVENT;
	mx->rchn = ESAI_RX_DMA_CTYPE;
	mx->sample_rate_min = SAMPLE_RATE_MIN;
	mx->sample_rate_max = SAMPLE_RATE_MAX;
	mx->main_clk_src = ESAI_MAIN_CLK_SRC;
#if defined(ESAI_CLK_MODE)
	mx->clk_mode = ESAI_CLK_MODE;
#else
	mx->clk_mode = ESAI_CLK_SLAVE;
#endif

#if defined(ESAI_NUM_SLOTS)
	mx->nslots = ESAI_NUM_SLOTS
#else	
	mx->nslots = 2;
#endif

#if defined(ESAI_SLOT_SIZE)
	mx->slot_size = ESAI_SLOT_SIZE;
#else
	mx->slot_size = 32;
#endif
#if defined(ESAI_MAIN_CLK_FREQ)
	 mx->main_clk_freq = ESAI_MAIN_CLK_FREQ;
#endif
#if defined(I2C_BUS_NUMBER)
	mx->i2c_bus = I2C_BUS_NUMBER;
#else
	mx->i2c_bus = -1;
#endif
#if defined(I2C_SLAVE_ADDR)
	mx->i2c_addr = I2C_SLAVE_ADDR;
#else
	mx->i2c_addr = -1;
#endif
#if defined SDO_PIN_MAP
	sdo_pin_map = SDO_PIN_MAP;
#else
	sdo_pin_map = "0";
#endif
#if defined SDI_PIN_MAP
	sdi_pin_map = SDI_PIN_MAP;
#else
	sdi_pin_map = "0";
#endif

#if defined ESAI_PROTOCOL
	mx->protocol = ESAI_PROTOCOL;
#else
	mx->protocol = PROTOCOL_I2S;
#endif
	configure_default_protocol_flags(mx);
	mx->log_enabled = 0;
	mx->rx_frag_size = mx->tx_frag_size = 4 * 1024;

#if defined ESAI_FS_ACTIVE_WIDTH
	mx->fs_active_width = ESAI_FS_ACTIVE_WIDTH;
#else
	mx->fs_active_width = FS_WORD;
#endif

#if defined SDO_PINS
	#warning "SDO_PINS is deprecated and should be replaced with SDO_PIN_MAP"
#endif

#if defined SDI_PINS
	#warning "SDI_PINS is deprecated and should be replaced with SDI_PIN_MAP"
#endif

	while (*args != '\0')
	{
		switch (getsubopt (&args, mxesai_opts, &value))
		{
			case OPT_ESAIBASE:
				mx->esaibase = strtoul (value, NULL, 0);
				break;
			case OPT_TEVT:
				mx->tevt = strtol (value, NULL, 0);
				break;
			case OPT_TCHN:
				mx->tchn = strtol (value, NULL, 0);
				break;
			case OPT_REVT:
				mx->revt = strtol (value, NULL, 0);
				break;
			case OPT_RCHN:
				mx->rchn = strtol (value, NULL, 0);
				break;
			case OPT_RATE:
				{
					char *value2;
					mx->sample_rate_min = mx->sample_rate_max = strtoul(value, 0, 0);
					if (ado_pcm_rate2flag(mx->sample_rate_min) == 0)
					{
						ado_error("Invalid sample rate - %d", mx->sample_rate_min);
						return EINVAL;
					}
					if ((value2 = strchr(value, ':')) != NULL)
					{
						mx->sample_rate_max = strtoul(value2 + 1, 0, 0);
						if (ado_pcm_rate2flag(mx->sample_rate_max) == 0)
						{
							ado_error("Invalid sample rate - %d", mx->sample_rate_max);
							return EINVAL;
						}
					}
				}
				break;
			case OPT_MAIN_CLK_SRC:
				mx->main_clk_src = strtol (value, NULL, 0);
				break;
			case OPT_MAIN_CLK_FREQ:
				mx->main_clk_freq = strtol (value, NULL, 0);
				break;
			case OPT_MIXER:
				if (strlen (value) > MAX_MIXEROPT)
				{
					ado_error ("MX ESAI: Board specific options pass maximum len %d", 
							MAX_MIXEROPT);
					ado_free (mx);
					return (-1);
				}
				strncat (mx->mixeropts, value, MAX_MIXEROPT);
				break;
			case OPT_RX_VOICES:
				if (value != NULL)
					rx_voices = strdup(value);
				break;
			case OPT_TX_VOICES:
				if (value != NULL)
					tx_voices = strdup(value);
				break;
			case OPT_I2C_BUS:
				mx->i2c_bus = strtol (value, NULL, 0);
				break;
			case OPT_I2C_ADDR:
				mx->i2c_addr = strtol (value, NULL, 0);
				break;
			case OPT_CLK_MODE:
				if (value && *value != '\0')
				{
					if (strcmp(value, "master") == 0)
					{
						mx->clk_mode = ESAI_CLK_MASTER;
						ado_debug (DB_LVL_DRIVER, "%s: Audio clock mode = Master", __FUNCTION__);
					}
					else if (strcmp(value, "slave") == 0)
					{
						mx->clk_mode = ESAI_CLK_SLAVE;
						ado_debug (DB_LVL_DRIVER, "%s: Audio clock mode = Slave", __FUNCTION__);
					}
				}
				break;
			case OPT_NSLOTS:
				mx->nslots = strtol (value, NULL, 0);
				break;
			case OPT_DEBUG:
				mx->log_enabled = 1;
				break;
			case OPT_RX_FRAG_SIZE:
				if (value && *value != '\0')
					mx->rx_frag_size = strtol (value, NULL, 0);
				break;
			case OPT_TX_FRAG_SIZE:
				if (value && *value != '\0')
					mx->tx_frag_size = strtol (value, NULL, 0);
				break;
			case OPT_PROTOCOL:
				if (value && *value != '\0')
				{
					if (strcmp(value, "i2s") == 0)
						mx->protocol = PROTOCOL_I2S;
					else
						mx->protocol = PROTOCOL_PCM;

					configure_default_protocol_flags(mx);
				}
				break;
			case OPT_BIT_DELAY:
				if (value && *value != '\0')
				{
					mx->bit_delay = atoi (value);
					if (mx->bit_delay > 1 || mx->bit_delay < 0)
					{
						ado_error ("Invalid bit_delay value (0 or 1)");
						return -EINVAL;
					}
				}
				break;
			case OPT_FSYNC_POL:
				if (value && *value != '\0')
				{
					mx->fsync_pol = atoi (value);
					if (mx->fsync_pol > 1 || mx->fsync_pol < 0)
					{
						ado_error ("Invalid fsync_pol value");
						return -EINVAL;
					}
				}
				break;
			case OPT_SLOT_SIZE:
				if (value && *value != '\0')
				{
					mx->slot_size = atoi (value);
					if (mx->slot_size != 16 && mx->slot_size != 32)
					{
						ado_error ("Invalid slot_size value (16 or 32)");
						return -EINVAL;
					}
				}
				break;
			case OPT_XCLK_POL:
				if (value && *value != '\0')
				{
					mx->xclk_pol = atoi (value);
					if (mx->xclk_pol > 1 || mx->xclk_pol < 0)
					{
						ado_error ("Invalid xclk_pol value");
						return -EINVAL;
					}
				}
				break;
			case OPT_RCLK_POL:
				if (value && *value != '\0')
				{
					mx->rclk_pol = atoi (value);
					if (mx->rclk_pol > 1 || mx->rclk_pol < 0)
					{
						ado_error ("Invalid rclk_pol value");
						return -EINVAL;
					}
				}
				break;
			case OPT_TX_CHMAP:
				if (value != NULL)
					sdo_chmap = strdup(value);
				break;
			case OPT_RX_CHMAP:
				if (value != NULL)
					sdi_chmap = strdup(value);
				break;
			case OPT_SDO_PINS:
				sdo_pin_map = strdup (value);
				break;
			case OPT_SDI_PINS:
				sdi_pin_map = strdup (value);
				break;
			case OPT_FS_ACTIVE_WIDTH:
				if (value && *value != '\0')
				{
					if (strcmp(value, "word") == 0)
						mx->fs_active_width = FS_WORD;
					else if (strcmp(value, "bit") == 0)
						mx->fs_active_width = FS_BIT;
				}
				break;

			default:
				ado_error ("MXESAI: Unsupported option '%s'", value);
				break;
		}
	}

	{
		char *value2;
		int idx;
		volatile int rx_cnt, tx_cnt;

		/* Determine the number of pins in use and convert to a bit-map */
		mx->num_sdo_pins = parse_pin_list(sdo_pin_map, &mx->sdo_pins);
		if (mx->num_sdo_pins > MAX_NUM_SDO_PINS)
		{
			ado_error ("MX ESAI: Cannot allocate more than %d SDO pins", MAX_NUM_SDO_PINS);
			return -1;
		}

		mx->num_sdi_pins = parse_pin_list(sdi_pin_map, &mx->sdi_pins);
		if (mx->num_sdi_pins > MAX_NUM_SDI_PINS)
		{
			ado_error ("MX ESAI: Cannot allocate more than %d SDI pins", MAX_NUM_SDI_PINS);
			return -1;
		}

		/* Count number of TX interfaces */
		value = (tx_voices == NULL) ? AIF_TX_VOICES : tx_voices;
		for (idx = 0, tx_cnt = 1; idx < strlen(value); (value[idx] == ':') ? tx_cnt++ : 0, idx++);
		/* Count number of RX interfaces */
		value = (rx_voices == NULL) ? AIF_RX_VOICES : rx_voices;
		for (idx = 0, rx_cnt = 1; idx < strlen(value); (value[idx] == ':') ? rx_cnt++ : 0, idx++);
		
		/* Allocate Audio interfaces */
		mx->num_tx_aif = tx_cnt;
		mx->num_rx_aif = rx_cnt;
		if ((mx->aif = (mxesai_aif_t *) ado_calloc (max(mx->num_tx_aif, mx->num_rx_aif), sizeof (mxesai_aif_t))) == NULL)
		{
			ado_error ("MX ESAI: Unable to allocate memory for mxesai (%s)", strerror (errno));
			return -1;
		}

		value = (tx_voices == NULL) ? AIF_TX_VOICES : tx_voices;

		mx->aif[0].play_strm.voices = strtoul(value, 0, 0);
		if (mx->aif[0].play_strm.voices > 8)
		{
			ado_debug(DB_LVL_DRIVER, "mxesai: Invalid tx[%d] channels %d", 0, mx->aif[0].play_strm.voices);
			if (tx_voices != NULL)
				free(tx_voices);
			if (rx_voices != NULL)
				free(rx_voices);
			ado_free(mx->aif);
			return (-1);
		}

		idx = 1;	/* Skip first character since we handled it above */
		while (idx < tx_cnt && (value2 = strchr(value, ':')) != NULL)
		{
			mx->aif[idx++].play_strm.voices = strtoul(value2 + 1, 0, 0);
			if (mx->aif[idx - 1].play_strm.voices > 8)
			{
				ado_debug(DB_LVL_DRIVER, "mxesai: Invalid tx channels %d", idx - 1,  mx->aif[idx - 1].play_strm.voices);
				if (tx_voices != NULL)
					free(tx_voices);
				if (rx_voices != NULL)
					free(rx_voices);
				ado_free(mx->aif);
				return (-1);
			}
			value = value2+1;
		}

		value = (rx_voices == NULL) ? AIF_RX_VOICES : rx_voices;

		mx->aif[0].cap_strm.voices = strtoul(value, 0, 0);
		if (mx->aif[0].cap_strm.voices > 8)
		{
			ado_debug(DB_LVL_DRIVER, "mxesai: Invalid rx[%d] channels %d", 0, mx->aif[0].cap_strm.voices);
			if (tx_voices != NULL)
				free(tx_voices);
			if (rx_voices != NULL)
				free(rx_voices);
			ado_free(mx->aif);
			return (-1);
		}

		idx = 1;	/* Skip first character since we handled it above */
		while (idx < rx_cnt && (value2 = strchr(value, ':')) != NULL)
		{
			mx->aif[idx++].cap_strm.voices = strtoul(value2 + 1, 0, 0);
			if (mx->aif[idx - 1].cap_strm.voices > 8)
			{
				ado_debug(DB_LVL_DRIVER, "mxesai: Invalid tx channels %d", idx - 1,  mx->aif[idx - 1].cap_strm.voices);
				if (tx_voices != NULL)
					free(tx_voices);
				if (rx_voices != NULL)
					free(rx_voices);
				ado_free(mx->aif);
				return (-1);
			}
			value = value2+1;
		}
		/* Free strdup'd strings */
		if (tx_voices != NULL)
			free(tx_voices);
		if (rx_voices != NULL)
			free(rx_voices);
	}

	if (mx->num_sdo_pins > 1)
	{
#if defined SDO_RECONSTITUTE_MAP
		value = (sdo_chmap == NULL) ? SDO_RECONSTITUTE_MAP : sdo_chmap;
#else
		value = sdo_chmap;
#endif
		if ((mx->sdo_chmap = set_reconstitute_map(mx, SND_PCM_CHANNEL_PLAYBACK, value)) == NULL)
		{
			if (sdo_chmap != NULL)
				free(sdo_chmap);
			ado_free(mx->aif);
			return (-1);
		}
		/* Free strdup'd string */
		if (sdo_chmap != NULL)
			free(sdo_chmap);
	}

	if (mx->num_sdi_pins > 1)
	{
#if defined SDI_RECONSTITUTE_MAP
		value = (sdi_chmap == NULL) ? SDI_RECONSTITUTE_MAP : sdi_chmap;
#else
		value = sdi_chmap;
#endif
		if ((mx->sdi_chmap = set_reconstitute_map(mx, SND_PCM_CHANNEL_CAPTURE, value)) == NULL)
		{
			if (sdi_chmap != NULL)
				free(sdi_chmap);
			if (mx->num_sdo_pins > 1)
				ado_free(mx->sdo_chmap);
			ado_free(mx->aif);
			return (-1);
		}
		/* Free strdup'd string */
		if (sdi_chmap != NULL)
			free(sdi_chmap);
	}

	if (mx->i2c_bus != -1)
	{
		if (strcmp (mx->mixeropts, "") != 0)
			strncat (mx->mixeropts, ":", MAX_MIXEROPT);
		snprintf(mx->mixeropts, MAX_MIXEROPT, "%sdev=%d", mx->mixeropts, mx->i2c_bus);
	}

	if (mx->i2c_addr != -1)
	{
		if (strcmp (mx->mixeropts, "") != 0)
			strncat (mx->mixeropts, ":", MAX_MIXEROPT);
		snprintf(mx->mixeropts, MAX_MIXEROPT, "%saddr=0x%x", mx->mixeropts, mx->i2c_addr);
	}

	if (strcmp (mx->mixeropts, "") != 0)
		strncat (mx->mixeropts, ":", MAX_MIXEROPT);
	snprintf(mx->mixeropts, MAX_MIXEROPT, "%srx_voices=%d", mx->mixeropts, mx->aif[0].cap_strm.voices);

	return EOK;
}

/* defaults if not set in variant.h */
#ifndef MXESAI_FRAME_RATE_LIST1
	#define MXESAI_FRAME_RATE_LIST1	{ SND_PCM_RATE_8000, SND_PCM_RATE_16000, SND_PCM_RATE_32000, SND_PCM_RATE_48000 }
#endif

#ifndef MXESAI_FRAME_RATE_LIST2
	#define MXESAI_FRAME_RATE_LIST2	{ SND_PCM_RATE_11025, SND_PCM_RATE_22050, SND_PCM_RATE_44100, SND_PCM_RATE_88200 }
#endif

ado_ctrl_dll_init_t ctrl_init;
int
ctrl_init (HW_CONTEXT_T ** hw_context, ado_card_t * card, char *args)
{
	int cnt = 0;
	mxesai_t * mx;
	char     str[100];
	int i;
	uint32_t rate;
	off_t physaddr;
	dma_driver_info_t sdma_info;

	uint32_t ratelist1[] = MXESAI_FRAME_RATE_LIST1;
	uint32_t ratelist2[] = MXESAI_FRAME_RATE_LIST2;

	ado_debug (DB_LVL_DRIVER, "CTRL_DLL_INIT: MXESAI");

	if ((mx = (mxesai_t *) ado_calloc (1, sizeof (mxesai_t))) == NULL)
	{
		ado_error ("MX ESAI: Unable to allocate memory for mxesai (%s)",
			strerror (errno));
		return -1;
	}
	*hw_context = mx;

	if (mxesai_parse_commandline(mx, args) != EOK)
	{
		ado_free(mx);
		return -1;
	}

	ado_card_set_shortname (card, "MXESAI");
	ado_card_set_longname (card, "Freescale i.MX ESAI", 0);

	mx->esai = mmap_device_memory (0, sizeof (esai_t),
		PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, mx->esaibase);
	if (mx->esai == MAP_FAILED)
	{
		ado_error ("MX ESAI: Unable to mmap ESAI (%s)", strerror (errno));
		if (mx->sdo_chmap != NULL)
			ado_free(mx->sdo_chmap);
		if (mx->sdi_chmap != NULL)
			ado_free(mx->sdi_chmap);
		ado_free(mx->aif);
		ado_free (mx);
		return -1;
	}

	ado_mutex_init (&mx->hw_lock);

	if (get_dmafuncs (&mx->sdmafuncs, sizeof (dma_functions_t)) == -1)
	{
		ado_error ("MX ESAI: Failed to get DMA lib functions");
		ado_mutex_destroy (&mx->hw_lock);
		munmap_device_memory (mx->esai, sizeof (esai_t));
		if (mx->sdo_chmap != NULL)
			ado_free(mx->sdo_chmap);
		if (mx->sdi_chmap != NULL)
			ado_free(mx->sdi_chmap);
		ado_free(mx->aif);
		ado_free (mx);
		return -1;
	}

	my_attach_pulse (&mx->aif[0].play_strm.pulse,
		&mx->aif[0].play_strm.sdma_event, mxesai_play_pulse_hdlr, mx);
	my_attach_pulse (&mx->aif[0].cap_strm.pulse,
		&mx->aif[0].cap_strm.sdma_event, mxesai_cap_pulse_hdlr, mx);

	if (mx->sdmafuncs.init (NULL) == -1)
	{
		ado_error ("MX ESAI: DMA init failed");
		my_detach_pulse (&mx->aif[0].cap_strm.pulse);
		my_detach_pulse (&mx->aif[0].play_strm.pulse);
		ado_mutex_destroy (&mx->hw_lock);
		munmap_device_memory (mx->esai, sizeof (esai_t));
		if (mx->sdo_chmap != NULL)
			ado_free(mx->sdo_chmap);
		if (mx->sdi_chmap != NULL)
			ado_free(mx->sdi_chmap);
		ado_free(mx->aif);
		ado_free (mx);
		return -1;
	}

	mx->sdmafuncs.driver_info(&sdma_info);
	/*
	 * DMA channel setup for Playback
	 * 1) watermark = must match the TX FIFO watermark in ESAI
	 * 2) eventnum = ESAI TX0 DMA event
	 * 3) fifopaddr = Physical address of ESAI TX0 FIFO
	 * 4) regen,contloop = DMA in repeat/loop mode so we only need to configure
	 *    the DMA transfer on channel acquire and not on every interrupt.
	 */
	mem_offset ((void *)&(mx->esai->etdr), NOFD, 1, &physaddr, 0);
	build_dma_string (str, physaddr, mx->tevt, FIFO_WATERMARK);

	ado_debug(DB_LVL_DRIVER, "MX ESAI: Playback sdma priority = %d", sdma_info.max_priority);
	mx->aif[0].play_strm.dma_chn =
		mx->sdmafuncs.channel_attach(str, &mx->aif[0].play_strm.sdma_event, &mx->tchn,
			sdma_info.max_priority, DMA_ATTACH_EVENT_PER_SEGMENT);

	if (mx->aif[0].play_strm.dma_chn == NULL)
	{
		ado_error ("MX ESAI: SDMA playback channel attach failed");
		ctrl_destroy(mx);
		return -1;
	}

	/*
	 * DMA channel setup for Capture
	 * 1) watermark = must match the RX FIFO watermark in ESAI
	 * 2) eventnum = ESAI RX0 DMA event
	 * 3) fifopaddr = Physical address of ESAI RX0 FIFO
	 * 4) regen,contloop = DMA in repeat/loop mode so we only need to configure
	 *    the DMA transfer on channel acquire and not on every interrupt.
	 */
	mem_offset ((void *)&(mx->esai->erdr), NOFD, 1, &physaddr, 0);
	build_dma_string (str, physaddr, mx->revt, FIFO_WATERMARK);

	ado_debug(DB_LVL_DRIVER, "MX ESAI: Capture sdma priority = %d", sdma_info.max_priority);
	mx->aif[0].cap_strm.dma_chn =
		mx->sdmafuncs.channel_attach(str, &mx->aif[0].cap_strm.sdma_event, &mx->rchn,
			sdma_info.max_priority, DMA_ATTACH_EVENT_PER_SEGMENT);

	if (mx->aif[0].cap_strm.dma_chn == NULL)
	{
		ado_error ("MX ESAI: SDMA capture channel attach failed");
		ctrl_destroy(mx);
		return -1;
	}

	if (mxesai_init (mx) == -1)
	{
		ado_error ("MX ESAI: Unable to initialize ESAI registers");
		ctrl_destroy(mx);
		return -1;
	}

	/* If multiple rx audio interfaces then allocate a ping-pong DMA buffer for capture.
	 * We will DMA into this buffer and copy the TDM slot data out of this buffer into the
	 * appropriate client buffers that are allocated in the aquire function.
	 */
	if(mx->num_rx_aif > 1)
	{
		dma_transfer_t tinfo;
		int frag_idx = 0;
		int dma_xfer_size = mx->rx_frag_size;
		/* Align the ping and pong buffers to the audio frame size */
		int alignment = SAMPLE_SIZE * mx->nslots * mx->num_sdi_pins;
		dma_xfer_size = mx->rx_frag_size / alignment * alignment;

		/* Multiply transfer size by two for ping-pong buffer */
		mx->capture_dmabuf.size = dma_xfer_size * 2;
		mx->capture_dmabuf.flags = ADO_SHM_DMA_SAFE | ADO_BUF_CACHE;
		if((mx->capture_dmabuf.addr = ado_shm_alloc(mx->capture_dmabuf.size, mx->capture_dmabuf.name, mx->capture_dmabuf.flags, &mx->capture_dmabuf.phys_addr)) == NULL)
		{
			ado_error ("MX ESAI: Unable to initialize ESAI registers");
			ctrl_destroy(mx);
			return -1;
		}

		/* Setup the DMA transfer */
		memset(&tinfo, 0, sizeof(tinfo));

		/* Allocate descriptor List */
		tinfo.dst_addrs = malloc( 2  * sizeof(dma_addr_t));

		for (frag_idx = 0; frag_idx < 2; frag_idx++)
		{
			tinfo.dst_addrs[frag_idx].paddr = mx->capture_dmabuf.phys_addr + (frag_idx * (mx->capture_dmabuf.size / 2));
			tinfo.dst_addrs[frag_idx].len = (mx->capture_dmabuf.size / 2);
		}
		tinfo.dst_fragments = 2;
		tinfo.xfer_unit_size = 16;					/* 16-bit sample size */
		tinfo.xfer_bytes = mx->capture_dmabuf.size;
		mx->sdmafuncs.setup_xfer (mx->aif[0].cap_strm.dma_chn, &tinfo);
		free(tinfo.dst_addrs);	
	}

	/* If multiple tx audio interfaces then allocate a ping-pong DMA buffer for playback.
	 * We will DMA from this buffer to the hardware and copy the TDM slot data to the 
	 * appropriate client buffers that are allocated in the aquire function.
	 */
	if(mx->num_tx_aif > 1)
	{
		dma_transfer_t tinfo;
		int frag_idx = 0;
		int dma_xfer_size;
		/* Align the ping and pong buffers to the audio frame size */
		int alignment = SAMPLE_SIZE * mx->nslots * mx->num_sdo_pins;
		dma_xfer_size = mx->tx_frag_size / alignment * alignment;

		/* Multiply transfer size by two for ping-pong buffer */
		mx->playback_dmabuf.size = dma_xfer_size * 2;
		mx->playback_dmabuf.flags = ADO_SHM_DMA_SAFE | ADO_BUF_CACHE;
		if((mx->playback_dmabuf.addr = ado_shm_alloc(mx->playback_dmabuf.size, mx->playback_dmabuf.name, mx->playback_dmabuf.flags, &mx->playback_dmabuf.phys_addr)) == NULL)
		{
			ado_error ("MX ESAI: Unable to initialize ESAI registers");
			ctrl_destroy(mx);
			return -1;
		}

		/* Setup the DMA transfer */
		memset(&tinfo, 0, sizeof(tinfo));

		/* Allocate descriptor List */
		tinfo.src_addrs = malloc( 2  * sizeof(dma_addr_t));

		for (frag_idx = 0; frag_idx < 2; frag_idx++)
		{
			tinfo.src_addrs[frag_idx].paddr = mx->playback_dmabuf.phys_addr + (frag_idx * (mx->playback_dmabuf.size / 2));
			tinfo.src_addrs[frag_idx].len = (mx->playback_dmabuf.size / 2);
		}
		tinfo.src_fragments = 2;
		tinfo.xfer_unit_size = 16;					/* 16-bit sample size */
		tinfo.xfer_bytes = mx->playback_dmabuf.size;
		mx->sdmafuncs.setup_xfer (mx->aif[0].play_strm.dma_chn, &tinfo);
		free(tinfo.src_addrs);	
	}

	// Define the various playback capabilities then copy to capture capabilities
	mx->aif[0].play_strm.pcm_caps.chn_flags =
		SND_PCM_CHNINFO_BLOCK | SND_PCM_CHNINFO_STREAM |
		SND_PCM_CHNINFO_INTERLEAVE | SND_PCM_CHNINFO_BLOCK_TRANSFER |
		SND_PCM_CHNINFO_MMAP | SND_PCM_CHNINFO_MMAP_VALID;
	mx->aif[0].play_strm.pcm_caps.formats = SND_PCM_FMT_S16_LE;


	if (mx->clk_mode == ESAI_CLK_MASTER)
	{
		uint32_t *ratelist;
		int cnt = 0;
		if ((mx->main_clk_freq % 48000) == 0)
		{
			ratelist = ratelist1;
			cnt = sizeof(ratelist1)/sizeof(ratelist1[0]);
		}
		else
		{
			ratelist = ratelist2;
			cnt = sizeof(ratelist2)/sizeof(ratelist2[0]);
		}

		for (i = 0; i < cnt; i++)
		{
			rate = ado_pcm_flag2rate(ratelist[i]);
			if (rate >= mx->sample_rate_min && rate <= mx->sample_rate_max)
				mx->aif[0].play_strm.pcm_caps.rates |= ratelist[i];
		}
	}
	else
	{
		mx->aif[0].play_strm.pcm_caps.rates = ado_pcm_rate2flag(mx->sample_rate_max);
	}

	mx->aif[0].play_strm.pcm_caps.min_rate = mx->sample_rate_min;
	mx->aif[0].play_strm.pcm_caps.max_rate = mx->sample_rate_max;
	mx->aif[0].play_strm.pcm_caps.min_voices = mx->aif[0].play_strm.voices;
	mx->aif[0].play_strm.pcm_caps.max_voices = mx->aif[0].play_strm.voices;
	mx->aif[0].play_strm.pcm_caps.min_fragsize = 64;
	mx->aif[0].play_strm.pcm_caps.max_fragsize = 32 * 1024;

	for (cnt = 1; cnt < mx->num_tx_aif; cnt++)
	{
		memcpy (&mx->aif[cnt].play_strm.pcm_caps, &mx->aif[0].play_strm.pcm_caps,
			sizeof (mx->aif[cnt].play_strm.pcm_caps));
		mx->aif[cnt].play_strm.pcm_caps.min_voices = mx->aif[cnt].play_strm.voices;
		mx->aif[cnt].play_strm.pcm_caps.max_voices = mx->aif[cnt].play_strm.voices;
	}

	for (cnt = 0; cnt < mx->num_rx_aif; cnt++)
	{
		memcpy (&mx->aif[cnt].cap_strm.pcm_caps, &mx->aif[0].play_strm.pcm_caps,
			sizeof (mx->aif[cnt].cap_strm.pcm_caps));
		mx->aif[cnt].cap_strm.pcm_caps.min_voices = mx->aif[cnt].cap_strm.voices;
		mx->aif[cnt].cap_strm.pcm_caps.max_voices = mx->aif[cnt].cap_strm.voices;
	}

	mx->aif[0].play_strm.pcm_funcs.capabilities2 = mxesai_capabilities;
	mx->aif[0].play_strm.pcm_funcs.aquire = mxesai_playback_aquire;
	mx->aif[0].play_strm.pcm_funcs.release = mxesai_playback_release;
	mx->aif[0].play_strm.pcm_funcs.prepare = mxesai_prepare;
	mx->aif[0].play_strm.pcm_funcs.trigger = mxesai_playback_trigger;
    /* If multiple output pins are used and there is only 1 tx_aif we need to use the reconstitute callout to shuffle channel order
     * Note: If more then 1 tx_aif is used then the mxesai_dmaplayback_combine() will call the reconstitute function directely.
     */
    if (mx->num_sdo_pins > 1 && mx->num_tx_aif == 1)
        mx->aif[0].play_strm.pcm_funcs.reconstitute2 = mxesai_playback_reconstitute;

    if (mx->num_sdo_pins > 1)
    {
        /* Allocate a playback reconstitue buffer (16bit samples) */
        if ((mx->sdo_reconstitute_buffer = ado_calloc ((mx->num_sdo_pins * mx->nslots), sizeof(int16_t))) == NULL)
        {
            ado_error ("MX ESAI: Unable to allocate playback reconstitute buffer (%s)", strerror (errno));
            ctrl_destroy(mx);
            return -1;
        }
    }

#if 0
	mx->aif[0].play_strm.pcm_funcs.position = mxesai_position;
#endif

	mx->aif[0].cap_strm.pcm_funcs.capabilities2 = mxesai_capabilities;
	mx->aif[0].cap_strm.pcm_funcs.aquire = mxesai_capture_aquire;
	mx->aif[0].cap_strm.pcm_funcs.release = mxesai_capture_release;
	mx->aif[0].cap_strm.pcm_funcs.prepare = mxesai_prepare;
	mx->aif[0].cap_strm.pcm_funcs.trigger = mxesai_capture_trigger;
    if (mx->num_sdi_pins > 1 && mx->num_rx_aif == 1)
        mx->aif[0].cap_strm.pcm_funcs.reconstitute2 = mxesai_capture_reconstitute;
    if (mx->num_sdi_pins > 1)
    {
        /* Allocate a capture reconstitue buffer (16bit samples) */
        if ((mx->sdi_reconstitute_buffer = ado_calloc ((mx->num_sdi_pins * mx->nslots), sizeof(int16_t))) == NULL)
        {
            ado_error ("MX ESAI: Unable to allocate capture reconstitute buffer (%s)", strerror (errno));
            ctrl_destroy(mx);
            return -1;
        }
    }

#if 0
	mx->aif[0].cap_strm.pcm_funcs.position = mxesai_position;
#endif

	for (cnt = 0; cnt < max(mx->num_tx_aif, mx->num_rx_aif); cnt++)
	{
		char pcm_name[_POSIX_NAME_MAX];
		char pcm_name2[_POSIX_NAME_MAX];
		sprintf(pcm_name, "mxesai PCM %d", cnt);
		sprintf(pcm_name2, "mxesai-%d", cnt);
		if (ado_pcm_create (card, pcm_name, 0, pcm_name2,
				cnt < mx->num_tx_aif ? 1: 0, 
				cnt < mx->num_tx_aif ? &mx->aif[cnt].play_strm.pcm_caps : NULL,
				cnt < mx->num_tx_aif ? &mx->aif[0].play_strm.pcm_funcs : NULL ,
				cnt < mx->num_rx_aif ? 1: 0, 
				cnt < mx->num_rx_aif ? &mx->aif[cnt].cap_strm.pcm_caps : NULL,
				cnt < mx->num_rx_aif ? &mx->aif[0].cap_strm.pcm_funcs : NULL,
				&mx->aif[cnt].pcm))
		{
			ado_error ("MX ESAI: Unable to create pcm devices (%s)", strerror (errno));
			ctrl_destroy(mx);
			return -1;
		}
		ado_debug(DB_LVL_DRIVER, "MX ESAI: PCM%d -> TX voices = %d, RX voices = %d", cnt, mx->aif[cnt].play_strm.voices, mx->aif[cnt].cap_strm.voices); 	
		/* Only create on mixer device for the card */
		if (cnt == 0)
		{
			if (codec_mixer (card, mx, mx->aif[cnt].pcm))
			{
				ado_error ("MX ESAI: Unable to create codec mixer");
				ctrl_destroy(mx);
				return -1;
			}
		}
		else if (cnt > 0 && cnt < mx->num_tx_aif)
		{
			if (ado_pcm_sw_mix (card, mx->aif[cnt].pcm, mx->mixer))
			{
				ado_error ("MX ESAI: Unable to create PCM software mixer %d", cnt);
				ctrl_destroy(mx);
				return -1;
			}
		}
	}
#if defined (VARIANT_mx6sabreARD) || defined (VARIANT_mx6q_ddr3) || defined (VARIANT_mx53sabre_ai) 
	if (mx->num_sdi_pins == 2)
	{
		/* Associate the second pcm device with the MIC_IN mixer group 
		 * The mixer assocation for the first pcm device is handled in codec_mixer().
		 */
		if (mx->mixer && mx->num_rx_aif > 1 && mx->aif[0].cap_strm.voices == 2)
		{
			ado_pcm_chn_mixer (mx->aif[1].pcm, ADO_PCM_CHANNEL_CAPTURE, mx->mixer,
				ado_mixer_find_element (mx->mixer, SND_MIXER_ETYPE_CAPTURE1,
					SND_MIXER_ELEMENT_CAPTURE, 0), ado_mixer_find_group (mx->mixer, SND_MIXER_MIC_IN, 0));
		}
	}
#endif
	return 0;
}

ado_ctrl_dll_destroy_t ctrl_destroy;
int
ctrl_destroy (HW_CONTEXT_T * mx)
{
	ado_debug (DB_LVL_DRIVER, "CTRL_DLL_DESTROY: MXESAI");
	if (mx->aif[0].cap_strm.dma_chn != NULL)
		mx->sdmafuncs.channel_release (mx->aif[0].cap_strm.dma_chn);
	if (mx->aif[0].play_strm.dma_chn)
		mx->sdmafuncs.channel_release (mx->aif[0].play_strm.dma_chn);
	my_detach_pulse (&mx->aif[0].cap_strm.pulse);
	my_detach_pulse (&mx->aif[0].play_strm.pulse);
	mx->sdmafuncs.fini ();
	if (mx->playback_dmabuf.addr != NULL)
	{
		/* Free global playback transfer buffer */
		ado_shm_free (mx->playback_dmabuf.addr, mx->playback_dmabuf.size, mx->playback_dmabuf.name);
	}
	if (mx->capture_dmabuf.addr != NULL)
	{
		/* Free global capture transfer buffer */
		ado_shm_free (mx->capture_dmabuf.addr, mx->capture_dmabuf.size, mx->capture_dmabuf.name);
	}

	ado_mutex_destroy (&mx->hw_lock);
	/* Disconnect and disable ESAI */
	mx->esai->prrc = 0x0;
	mx->esai->pcrc = 0x0;
	mx->esai->ecr = 0x0;
	munmap_device_memory (mx->esai, sizeof (esai_t));

	if (mx->sdo_reconstitute_buffer != NULL)
		ado_free(mx->sdo_reconstitute_buffer);
	if (mx->sdo_chmap != NULL)
		ado_free(mx->sdo_chmap);
	if (mx->sdi_reconstitute_buffer != NULL)
		ado_free(mx->sdi_reconstitute_buffer);
	if (mx->sdi_chmap != NULL)
		ado_free(mx->sdi_chmap);

	ado_free(mx->aif);
	ado_free (mx);

	return 0;
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/deva/ctrl/mxesai/mxesai_dll.c $ $Rev: 808121 $")
#endif
