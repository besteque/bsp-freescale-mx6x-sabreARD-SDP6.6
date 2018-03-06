/*
 * $QNXLicenseC: 
 * Copyright 2009, 2011 QNX Software Systems.  
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

#include <audio_driver.h>
#include <string.h>

#define	 MIXER_CONTEXT_T cs42448_context_t

#include <hw/i2c.h>
#include "cs42448.h"

#define LOCAL static

#define DEFAULT_I2C_NUM         0	/* Default I2C device number */
#define DEFAULT_I2C_ADDR        0x48	/* Slave addr of CS42448 Codec */
#define DEFAULT_CLK_RATIO       512	/* Default ratio between master clock and frame clock */
#define DEFAULT_MODE            "slave"	/* SOC is slave by default (i.e. the codec is the master) */
#define FORMAT_I2S              0
#define FORMAT_LJ               1
#define MAX_STRLEN              50

typedef struct
{
	int	fd;
	int i2c_num;
	int	addr;
	int	play_strm_voices;
	int	cap_strm_voices;
	int	mode_bits;
	char	mode[MAX_STRLEN];
	int	format;
	uint32_t ain_mute[4];
	uint8_t ain_vol[4];
} cs42448_context_t;

static int32_t pcm_devices[1] = {
	0
};

static snd_mixer_voice_t stereo_voices[2] = {
	{SND_MIXER_VOICE_LEFT, 0},
	{SND_MIXER_VOICE_RIGHT, 0}
};

static snd_mixer_voice_t quad_voices[4] = {
	{SND_MIXER_VOICE_LEFT, 0},
	{SND_MIXER_VOICE_RIGHT, 0},
	{SND_MIXER_VOICE_REAR_LEFT, 0},
	{SND_MIXER_VOICE_REAR_RIGHT, 0}
};

static snd_mixer_voice_t eight_voices[8] = {
	{SND_MIXER_VOICE_LEFT, 0},
	{SND_MIXER_VOICE_RIGHT, 0},
	{SND_MIXER_VOICE_CENTER, 0},
	{SND_MIXER_VOICE_WOOFER, 0},
	{SND_MIXER_VOICE_REAR_LEFT, 0},
	{SND_MIXER_VOICE_REAR_RIGHT, 0},
	{SND_MIXER_VOICE_SURR_LEFT, 0},
	{SND_MIXER_VOICE_SURR_RIGHT, 0}
};

static struct snd_mixer_element_volume1_range output_range[8] = {
	{0, 255, -12750, 0},
	{0, 255, -12750, 0},
	{0, 255, -12750, 0},
	{0, 255, -12750, 0},
	{0, 255, -12750, 0},
	{0, 255, -12750, 0},
	{0, 255, -12750, 0},
	{0, 255, -12750, 0}

};

static struct snd_mixer_element_volume1_range input_range[4] = {
	{0, 176, -6400, 2400},
	{0, 176, -6400, 2400},
	{0, 176, -6400, 2400},
	{0, 176, -6400, 2400},
};

static int
cs42448_write (MIXER_CONTEXT_T * cs42448, uint8_t register_address, uint8_t val, uint8_t bitmask)
{
	struct send
	{
		i2c_send_t hdr;
		uint8_t buf[2];
	} cs42448_wr_data;

	struct send_recv
	{
		i2c_sendrecv_t hdr;
		uint8_t buf[2];
	} cs42448_rd_data;

	uint8_t data_tmp;

	cs42448_rd_data.buf[0] = register_address;
	cs42448_rd_data.hdr.send_len = 1;
	cs42448_rd_data.hdr.recv_len = 1;
	cs42448_rd_data.hdr.slave.addr = cs42448->addr;
	cs42448_rd_data.hdr.slave.fmt = I2C_ADDRFMT_7BIT;
	cs42448_rd_data.hdr.stop = 0;

	if (devctl (cs42448->fd, DCMD_I2C_SENDRECV, &cs42448_rd_data, sizeof (cs42448_rd_data), NULL))
	{
		ado_error ("Failed to read codec reg values: %s\n", strerror (errno));
		return -1;
	}

	data_tmp = cs42448_rd_data.buf[0];

	/* Now that the register value is read, Set and clear the required bits only */
	cs42448_wr_data.buf[0] = register_address;
	cs42448_wr_data.buf[1] = (data_tmp & ~bitmask) | (val & bitmask);

	cs42448_wr_data.hdr.len = 2;
	cs42448_wr_data.hdr.slave.addr = cs42448->addr;
	cs42448_wr_data.hdr.slave.fmt = I2C_ADDRFMT_7BIT;
	cs42448_wr_data.hdr.stop = 1;

	if (devctl (cs42448->fd, DCMD_I2C_SEND, &cs42448_wr_data, sizeof (cs42448_wr_data), NULL))
	{
		ado_error ("Failed to write to codec: %s\n", strerror (errno));
		return -1;
	}

	return 0;
}

static  uint8_t
cs42448_read (MIXER_CONTEXT_T * cs42448, uint8_t register_address)
{
	struct send_recv
	{
		i2c_sendrecv_t hdr;
		uint8_t buf[2];
	} cs42448_rd_data;

	/*Read the Registers Current Value */
	cs42448_rd_data.buf[0] = register_address;
	cs42448_rd_data.hdr.send_len = 1;
	cs42448_rd_data.hdr.recv_len = 1;

	cs42448_rd_data.hdr.slave.addr = cs42448->addr;
	cs42448_rd_data.hdr.slave.fmt = I2C_ADDRFMT_7BIT;
	cs42448_rd_data.hdr.stop = 1;

	if (devctl (cs42448->fd, DCMD_I2C_SENDRECV, &cs42448_rd_data, sizeof (cs42448_rd_data), NULL))
		ado_error ("Failed to write to codec: %s\n", strerror (errno));
	
	return cs42448_rd_data.buf[0];
}

static  int32_t
cs42448_ainx_mute_control (MIXER_CONTEXT_T * cs42448, ado_mixer_delement_t * element, uint8_t set,
	uint32_t * val, void *instance_data)
{
	int32_t altered = 0;
	uint32_t ainx_vol_ctl = (uint32_t) instance_data;
	uint8_t vol_idx = (ainx_vol_ctl == AIN0_VOL_CTL) ? 0 : 2;
	uint8_t mute_idx = (ainx_vol_ctl == AIN0_VOL_CTL) ? 0 : 1;

	if (set)
	{
		altered = cs42448->ain_mute[mute_idx] != val[0] ? 1 : 0;
		if (altered)
		{
			if (val[0] > cs42448->ain_mute[mute_idx])
			{
				/* 0x80 == -64dB closest to mute we can get via volume controls */
				if (!(cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_FRONT_LEFT) && (val[0] & SND_MIXER_CHN_MASK_FRONT_LEFT))
				{
					cs42448->ain_mute[mute_idx] |= SND_MIXER_CHN_MASK_FRONT_LEFT;
					cs42448_write (cs42448, ainx_vol_ctl + 0, 0x80, 0xff);
				}
				if (!(cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_FRONT_RIGHT) && (val[0] & SND_MIXER_CHN_MASK_FRONT_RIGHT))
				{
					cs42448->ain_mute[mute_idx] |= SND_MIXER_CHN_MASK_FRONT_RIGHT;
					cs42448_write (cs42448, ainx_vol_ctl + 1, 0x80, 0xff);
				}

				if (cs42448->cap_strm_voices == 4)
				{
					/* 0x80 == -64dB closest to mute we can get via volume controls */
					if (!(cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_REAR_LEFT) && (val[0] & SND_MIXER_CHN_MASK_REAR_LEFT))
					{
						cs42448->ain_mute[mute_idx] |= SND_MIXER_CHN_MASK_REAR_LEFT;
						cs42448_write (cs42448, ainx_vol_ctl + 2, 0x80, 0xff);
					}
					if (!(cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_REAR_RIGHT) && (val[0] & SND_MIXER_CHN_MASK_REAR_RIGHT))
					{
						cs42448->ain_mute[mute_idx] |= SND_MIXER_CHN_MASK_REAR_RIGHT;
						cs42448_write (cs42448, ainx_vol_ctl + 3, 0x80, 0xff);
					}
				}
			}
			else
			{
				/* Restore configured volume level */
				if ((cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_FRONT_LEFT) && !(val[0] & SND_MIXER_CHN_MASK_FRONT_LEFT))
				{
					cs42448->ain_mute[mute_idx] &= ~SND_MIXER_CHN_MASK_FRONT_LEFT;
					cs42448_write (cs42448, ainx_vol_ctl + 0, cs42448->ain_vol[vol_idx], 0xff);
				}
				if ((cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_FRONT_RIGHT) && !(val[0] & SND_MIXER_CHN_MASK_FRONT_RIGHT))
				{
					cs42448->ain_mute[mute_idx] &= ~SND_MIXER_CHN_MASK_FRONT_RIGHT;
					cs42448_write (cs42448, ainx_vol_ctl + 1, cs42448->ain_vol[vol_idx + 1], 0xff);
				}
				
				if (cs42448->cap_strm_voices == 4)
				{
					/* 0x80 == -64dB closest to mute we can get via volume controls */
					if ((cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_REAR_LEFT) && !(val[0] & SND_MIXER_CHN_MASK_REAR_LEFT))
					{
						cs42448->ain_mute[mute_idx] &= ~SND_MIXER_CHN_MASK_REAR_LEFT;
						cs42448_write (cs42448, ainx_vol_ctl + 2, cs42448->ain_vol[vol_idx + 2], 0xff);
					}
					if ((cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_REAR_RIGHT) && !(val[0] & SND_MIXER_CHN_MASK_REAR_RIGHT))
					{
						cs42448->ain_mute[mute_idx] &= ~SND_MIXER_CHN_MASK_REAR_RIGHT;
						cs42448_write (cs42448, ainx_vol_ctl + 3, cs42448->ain_vol[vol_idx + 3], 0xff);
					}
				}
			}
		}
	}
	else
	{
		val[0] = cs42448->ain_mute[mute_idx];
	}

	return altered;
}

static  int32_t
cs42448_ainx_vol_control (MIXER_CONTEXT_T * cs42448, ado_mixer_delement_t * element, uint8_t set,
	uint32_t * vol, void *instance_data)
{
	int32_t altered = 0;
	uint32_t ainx_vol_ctl = (uint32_t) instance_data;
	uint8_t vol_idx = (ainx_vol_ctl == AIN0_VOL_CTL) ? 0 : 2;
	uint8_t mute_idx = (ainx_vol_ctl == AIN0_VOL_CTL) ? 0 : 1;

	if (set)
	{
		vol[SND_MIXER_CHN_FRONT_LEFT] = vol[SND_MIXER_CHN_FRONT_LEFT] >= 128 ? vol[SND_MIXER_CHN_FRONT_LEFT] - 128 : vol[SND_MIXER_CHN_FRONT_LEFT] + 128;
		vol[SND_MIXER_CHN_FRONT_RIGHT] = vol[SND_MIXER_CHN_FRONT_RIGHT] >= 128 ? vol[SND_MIXER_CHN_FRONT_RIGHT] - 128 : vol[SND_MIXER_CHN_FRONT_RIGHT] + 128;
		altered = vol[SND_MIXER_CHN_FRONT_LEFT] != cs42448->ain_vol[vol_idx] || vol[SND_MIXER_CHN_FRONT_RIGHT] != cs42448->ain_vol[vol_idx + 1];
		if (cs42448->cap_strm_voices == 4)
		{
			vol[SND_MIXER_CHN_REAR_LEFT] = vol[SND_MIXER_CHN_REAR_LEFT] >= 128 ? vol[SND_MIXER_CHN_REAR_LEFT] - 128 : vol[SND_MIXER_CHN_REAR_LEFT] + 128;
			vol[SND_MIXER_CHN_REAR_RIGHT] = vol[SND_MIXER_CHN_REAR_RIGHT] >= 128 ? vol[SND_MIXER_CHN_REAR_RIGHT] - 128 : vol[SND_MIXER_CHN_REAR_RIGHT] + 128;
			altered |= vol[SND_MIXER_CHN_REAR_LEFT] != cs42448->ain_vol[vol_idx + 2] || vol[SND_MIXER_CHN_REAR_RIGHT] != cs42448->ain_vol[vol_idx + 3];
		}

		if (altered)
		{
			cs42448->ain_vol[vol_idx] = vol[SND_MIXER_CHN_FRONT_LEFT];
			cs42448->ain_vol[vol_idx + 1] = vol[SND_MIXER_CHN_FRONT_RIGHT];
			/* set volume */
			if (!(cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_FRONT_LEFT))
				cs42448_write (cs42448, ainx_vol_ctl + 0, vol[SND_MIXER_CHN_FRONT_LEFT], 0xff);
			if (!(cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_FRONT_RIGHT))
				cs42448_write (cs42448, ainx_vol_ctl + 1, vol[SND_MIXER_CHN_FRONT_RIGHT], 0xff);
		
			if (cs42448->cap_strm_voices == 4)
			{
				cs42448->ain_vol[vol_idx + 2] = vol[SND_MIXER_CHN_REAR_LEFT];
				cs42448->ain_vol[vol_idx + 3] = vol[SND_MIXER_CHN_REAR_RIGHT];
				/* set volume */
				if (!(cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_REAR_LEFT))
					cs42448_write (cs42448, ainx_vol_ctl + 2, vol[SND_MIXER_CHN_REAR_LEFT], 0xff);
				if (!(cs42448->ain_mute[mute_idx] & SND_MIXER_CHN_MASK_REAR_RIGHT))
					cs42448_write (cs42448, ainx_vol_ctl + 3, vol[SND_MIXER_CHN_REAR_RIGHT], 0xff);
			}
		}
	}	
	else
	{
		vol[SND_MIXER_CHN_FRONT_LEFT] = cs42448->ain_vol[vol_idx] >= 128 ? cs42448->ain_vol[vol_idx] - 128 : cs42448->ain_vol[vol_idx] + 128;
		vol[SND_MIXER_CHN_FRONT_RIGHT] = cs42448->ain_vol[vol_idx + 1] >= 128 ? cs42448->ain_vol[vol_idx + 1] - 128 : cs42448->ain_vol[vol_idx + 1] + 128;
		if (cs42448->cap_strm_voices == 4)
		{
			vol[SND_MIXER_CHN_REAR_LEFT] = cs42448->ain_vol[vol_idx + 2] >= 128 ? cs42448->ain_vol[vol_idx + 2] - 128 : cs42448->ain_vol[vol_idx + 2] + 128;
			vol[SND_MIXER_CHN_REAR_RIGHT] = cs42448->ain_vol[vol_idx + 3] >= 128 ? cs42448->ain_vol[vol_idx + 3] - 128 : cs42448->ain_vol[vol_idx + 3] + 128;
		}
	}
	return altered;
}

static  int32_t
cs42448_aoutx_mute_control (MIXER_CONTEXT_T * cs42448, ado_mixer_delement_t * element, uint8_t set,
	uint32_t * val, void *instance_data)
{
	int32_t altered = 0;
	uint32_t mute_mask = (uint32_t) instance_data;
	uint32_t aoutx_mute;
	uint32_t data;
	
	aoutx_mute = cs42448_read (cs42448, DAC_CH_MUTE) & mute_mask;
	
	if (set)
	{
		val[0] &= (SND_MIXER_CHN_MASK_FRONT_LEFT | SND_MIXER_CHN_MASK_FRONT_RIGHT |
				   SND_MIXER_CHN_MASK_FRONT_CENTER | SND_MIXER_CHN_MASK_WOOFER |
				   SND_MIXER_CHN_MASK_REAR_LEFT | SND_MIXER_CHN_MASK_REAR_RIGHT |
				   SND_MIXER_CHN_MASK_SURR_LEFT | SND_MIXER_CHN_MASK_SURR_RIGHT);
		
		/* Re-format val[0] to match the mute bitmask of the codec
		 * (move the woofer to be before the rears rather then after the rears) 
		 */
		data = (val[0] & (SND_MIXER_CHN_MASK_FRONT_LEFT | SND_MIXER_CHN_MASK_FRONT_RIGHT | SND_MIXER_CHN_MASK_FRONT_CENTER | SND_MIXER_CHN_MASK_SURR_LEFT | SND_MIXER_CHN_MASK_SURR_RIGHT));
		data |= ((val[0] & SND_MIXER_CHN_MASK_WOOFER) >> 2);
		data |= ((val[0] & (SND_MIXER_CHN_MASK_REAR_LEFT | SND_MIXER_CHN_MASK_REAR_RIGHT)) << 1);
		
		altered = data != aoutx_mute;
		if (altered)
			cs42448_write (cs42448, DAC_CH_MUTE, data, mute_mask);
	}
	else
	{
		/* Re-format codec mute bitmap to conform to io-audio's mute bitmap.
		 * (shift the woofer out after the rears) 
		 */
		data = (aoutx_mute & (AOUT1_MUTE | AOUT2_MUTE | AOUT3_MUTE | AOUT7_MUTE | AOUT8_MUTE));
		data |= ((aoutx_mute & AOUT4_MUTE) << 2);
		data |= ((aoutx_mute & (AOUT5_MUTE | AOUT6_MUTE)) >> 1);
	
		val[0] = data;
	}

	return altered;
}

static  int32_t
cs42448_aoutx_vol_control (MIXER_CONTEXT_T * cs42448, ado_mixer_delement_t * element, uint8_t set,
	uint32_t * vol, void *instance_data)
{
	int32_t altered = 0;
	uint8_t data[8];
	uint32_t aoutx_vol_ctl = (uint32_t) instance_data;

	/* read volume */
	data[SND_MIXER_CHN_FRONT_LEFT]   = 255 - cs42448_read (cs42448, aoutx_vol_ctl + 0);
	data[SND_MIXER_CHN_FRONT_RIGHT]  = 255 - cs42448_read (cs42448, aoutx_vol_ctl + 1);
	data[SND_MIXER_CHN_FRONT_CENTER] = 255 - cs42448_read (cs42448, aoutx_vol_ctl + 2);
	data[SND_MIXER_CHN_WOOFER]       = 255 - cs42448_read (cs42448, aoutx_vol_ctl + 3);
	data[SND_MIXER_CHN_REAR_LEFT]    = 255 - cs42448_read (cs42448, aoutx_vol_ctl + 4);
	data[SND_MIXER_CHN_REAR_RIGHT]   = 255 - cs42448_read (cs42448, aoutx_vol_ctl + 5);
	data[SND_MIXER_CHN_SURR_LEFT]    = 255 - cs42448_read (cs42448, aoutx_vol_ctl + 6);
	data[SND_MIXER_CHN_SURR_RIGHT]   = 255 - cs42448_read (cs42448, aoutx_vol_ctl + 7);
	
	switch (cs42448->play_strm_voices)
	{
	/* we only support 7.1 (8) channel playback */
	case 8:
		if (set)
		{
			altered = vol[0] != data[0] || vol[1] != data[1] || vol[2] != data[2] ||
				vol[3] != data[3] || vol[4] != data[4] || vol[5] != data[5] || 
				vol[6] != data[6] || vol[7] != data[7] ;
			
			if (altered)
			{	
				/* set volume */
				cs42448_write (cs42448, aoutx_vol_ctl + 0, 255 - vol[SND_MIXER_CHN_FRONT_LEFT], 0xff);
				cs42448_write (cs42448, aoutx_vol_ctl + 1, 255 - vol[SND_MIXER_CHN_FRONT_RIGHT], 0xff);
				cs42448_write (cs42448, aoutx_vol_ctl + 2, 255 - vol[SND_MIXER_CHN_FRONT_CENTER], 0xff);
				cs42448_write (cs42448, aoutx_vol_ctl + 3, 255 - vol[SND_MIXER_CHN_WOOFER], 0xff);
				cs42448_write (cs42448, aoutx_vol_ctl + 4, 255 - vol[SND_MIXER_CHN_REAR_LEFT], 0xff);
				cs42448_write (cs42448, aoutx_vol_ctl + 5, 255 - vol[SND_MIXER_CHN_REAR_RIGHT], 0xff);
				cs42448_write (cs42448, aoutx_vol_ctl + 6, 255 - vol[SND_MIXER_CHN_SURR_LEFT], 0xff);
				cs42448_write (cs42448, aoutx_vol_ctl + 7, 255 - vol[SND_MIXER_CHN_SURR_RIGHT], 0xff);
			}
		}
		else
		{
			vol[0] = data[0];
			vol[1] = data[1];
			vol[2] = data[2];
			vol[3] = data[3];
			vol[4] = data[4];
			vol[5] = data[5];
			vol[6] = data[6];
			vol[7] = data[7];
		}
		break;

	default:
		ado_error("%s: Only support 8 channel playback. Not support play_strm_voices=%d\n", __func__, cs42448->play_strm_voices);
		break;
	}
	return altered;
}

/* Required for compatibility with Audioman
 * This switch is called by audio manager to ask deva to send the current HW status, i.e., whether headset is connected
 */
static int32_t
cs42448_audioman_refresh_set(MIXER_CONTEXT_T * hw_ctx, ado_dswitch_t * dswitch, snd_switch_t * cswitch,	void *instance_data)
{
	return (EOK);
}

static int32_t
cs42448_audioman_refresh_get(MIXER_CONTEXT_T * hw_ctx, ado_dswitch_t * dswitch, snd_switch_t * cswitch,	void *instance_data)
{
	/* Always return disabled as this switch does not maintain state */
	cswitch->type = SND_SW_TYPE_BOOLEAN;
	cswitch->value.enable = 0;
	return 0;
}

static  int32_t
cs42448_mixer_build (MIXER_CONTEXT_T * cs42448, ado_mixer_t * mixer)
{
	int     val;
	uint32_t chn_mask = 0;

	ado_mixer_delement_t *play_vol;
	ado_mixer_delement_t *play_mute;
	ado_mixer_delement_t *capture_vol;
	ado_mixer_delement_t *capture_mute;

	ado_mixer_dgroup_t *play_grp;
	ado_mixer_dgroup_t *capture_grp;

	int     error = 0;
	ado_mixer_delement_t *pre_elem = NULL, *elem = NULL;

	/*#################### */
	/*## Playback Group ## */
	/*#################### */
	
	/* Hardcode play channels to be 8 */
	val = 0xff;
	chn_mask = (SND_MIXER_CHN_MASK_7_1);

	if (!error && (pre_elem = ado_mixer_element_pcm1 (mixer, SND_MIXER_ELEMENT_PLAYBACK,
				SND_MIXER_ETYPE_PLAYBACK1, 1, &pcm_devices[0])) == NULL)
		error++;

	if (!error && (elem = ado_mixer_element_volume1 (mixer, "PCM Volume",
				cs42448->play_strm_voices, output_range, 
				(void *)cs42448_aoutx_vol_control,
				(void *) AOUT0_VOL_CTL, NULL)) == NULL)
		error++;

	if (!error && ado_mixer_element_route_add (mixer, pre_elem, elem) != 0)
		error++;

	play_vol = elem;
	pre_elem = elem;

	if (!error &&
		(elem =
			ado_mixer_element_sw1 (mixer, "PCM Mute", cs42448->play_strm_voices,
				(void *)cs42448_aoutx_mute_control, (void *) val, NULL)) == NULL)
		error++;

	if (!error && ado_mixer_element_route_add (mixer, pre_elem, elem) != 0)
		error++;

	play_mute = elem;
	pre_elem = elem;

	if (!error && (elem = ado_mixer_element_io (mixer, "PCM OUT",
				SND_MIXER_ETYPE_OUTPUT, 0, cs42448->play_strm_voices, eight_voices)) == NULL)
		error++;

	if (!error && ado_mixer_element_route_add (mixer, pre_elem, elem) != 0)
		error++;

	if (!error && (play_grp = ado_mixer_playback_group_create (mixer, SND_MIXER_PCM_OUT,
				chn_mask, play_vol, play_mute)) == NULL)
		error++;

	/*###################*/
	/*## Capture Group ##*/
	/*###################*/
	if (cs42448->cap_strm_voices == 4)
		chn_mask = SND_MIXER_CHN_MASK_4;
	else
		chn_mask = SND_MIXER_CHN_MASK_STEREO;

	if (!error && (pre_elem = ado_mixer_element_pcm1 (mixer, SND_MIXER_ELEMENT_CAPTURE,
				SND_MIXER_ETYPE_CAPTURE1, 1, &pcm_devices[0])) == NULL)
		error++;

	if (!error && (elem = ado_mixer_element_volume1 (mixer, cs42448->cap_strm_voices == 4 ? "PCM In Volume" : "Line In Volume",
			cs42448->cap_strm_voices, input_range, (void *)cs42448_ainx_vol_control, (void *) AIN0_VOL_CTL, NULL)) == NULL)
		error++;

	if (!error && ado_mixer_element_route_add (mixer, pre_elem, elem) != 0)
		error++;

	capture_vol = elem;
	pre_elem = elem;

	if (!error && (elem = ado_mixer_element_sw1 (mixer, cs42448->cap_strm_voices == 4 ? "PCM In Mute" : "Line In Mute",
			cs42448->cap_strm_voices, (void *)cs42448_ainx_mute_control, (void *) AIN0_VOL_CTL, NULL)) == NULL)
		error++;

	if (!error && ado_mixer_element_route_add (mixer, pre_elem, elem) != 0)
		error++;

	capture_mute = elem;
	pre_elem = elem;

	if (!error && (elem = ado_mixer_element_io (mixer, cs42448->cap_strm_voices == 4 ? "PCMIn" : "LineIn",
			SND_MIXER_ETYPE_INPUT, 0, cs42448->cap_strm_voices, cs42448->cap_strm_voices == 4 ? quad_voices : stereo_voices)) == NULL)
		error++;

	if (!error && ado_mixer_element_route_add (mixer, pre_elem, elem) != 0)
		error++;

	if (!error && (capture_grp = ado_mixer_capture_group_create (mixer, cs42448->cap_strm_voices == 4 ? SND_MIXER_PCM_IN : SND_MIXER_LINE_IN,
				chn_mask, capture_vol, capture_mute, NULL, NULL)) == NULL)
		error++;
	
	if (cs42448->cap_strm_voices == 2)
	{
		if (!error && (elem = ado_mixer_element_volume1 (mixer, "Mic In Volume",
				cs42448->cap_strm_voices, input_range, (void *)cs42448_ainx_vol_control, (void *) AIN2_VOL_CTL, NULL)) == NULL)
			error++;

		if (!error && ado_mixer_element_route_add (mixer, pre_elem, elem) != 0)
			error++;

		capture_vol = elem;
		pre_elem = elem;

		if (!error && (elem = ado_mixer_element_sw1 (mixer, "Mic In Mute", cs42448->cap_strm_voices, (void *)cs42448_ainx_mute_control,
				(void *)AIN2_VOL_CTL, NULL)) == NULL)
			error++;

		if (!error && ado_mixer_element_route_add (mixer, pre_elem, elem) != 0)
			error++;

		capture_mute = elem;
		pre_elem = elem;

		if (!error && (elem = ado_mixer_element_io (mixer, "MicIn",	SND_MIXER_ETYPE_INPUT, 0, cs42448->cap_strm_voices, stereo_voices)) == NULL)
			error++;

		if (!error && ado_mixer_element_route_add (mixer, pre_elem, elem) != 0)
			error++;

		if (!error && (capture_grp = ado_mixer_capture_group_create (mixer, SND_MIXER_MIC_IN,
					chn_mask, capture_vol, capture_mute, NULL, NULL)) == NULL)
			error++;

	}

	/* ########### */
	/* SWITCHES    */
	/* ########### */
	if (ado_mixer_switch_new(mixer, "Audioman Refresh", SND_SW_TYPE_BOOLEAN, 0, (void*)cs42448_audioman_refresh_get,
			(void *)cs42448_audioman_refresh_set, NULL, NULL) == NULL)
		return (-1);

	return (!error ? 0 : -1);
}

LOCAL char *cs42448_opts[] = {
	"dev",			// 0
	"addr",			// 1
	"clk_ratio",	// 2
	"mode",			// 3
	"format",		// 4
	"rx_voices",    // 5 
	"info",			// 6
	NULL
};

LOCAL char *opt_help[] = {
	"=[#]  : control device, default /dev/i2c0",
	"=[#]  : device address, default 0x48",
	"=[#]  : ratio between master clock, frame clock, default 512",
	"=[#]  : whether the SOC is master or slave",
	"=[#]  : format, e.g. I2S, LJ, etc.",
	"=[#]  : Number of capture voices/channels",
	"=[#]  : info",
	NULL
};

static uint32_t get_mode_bits(int ratio)
{
	switch (ratio)
	{
		case 256:
			return 0x0;
		case 384:
			return 0x2;
		case 512:
			return 0x4;
		case 768:
			return 0x6;
		case 1024:
			return 0x8;
		default:
			return NULL;
		
	}

}
/* Parse mixer options */
LOCAL int 
cs42448_parse_options (MIXER_CONTEXT_T * cs42448, char * args)
{
	char * value = NULL;
	int    opt;
	int    idx;
	char *p;

	// set default ratio between master and frame clock
	cs42448->mode_bits = get_mode_bits(DEFAULT_CLK_RATIO);
	
	// SOC is slave by default (codec is master)
	strncpy(cs42448->mode, DEFAULT_MODE, MAX_STRLEN);

	// format is I2S by default
	cs42448->format = FORMAT_I2S;

	while ((p = strchr(args, ':')) != NULL)
		*p = ',';

	while (*args != '\0')
	{
		opt = getsubopt (&args, cs42448_opts, &value);
		switch (opt) {
			case 0:
				if (value == NULL) 
					ado_error ("CS42448: Must specify a i2c device number");
				else
					cs42448->i2c_num = strtoul (value, NULL, 0);
				break;
			case 1:
				if (value == NULL) 
					ado_error ("CS42448: Must specify a value to addr");
				else
					cs42448->addr = strtoul (value, NULL, 0);
				break;
			case 2:
				if (value !=  NULL)
					cs42448->mode_bits = get_mode_bits(strtoul (value, NULL, 0));
					if (cs42448->mode_bits == NULL)
					{
						ado_error ("CS42448: invalid clock ratio, using default ratio (512)");
						cs42448->mode_bits = get_mode_bits(DEFAULT_CLK_RATIO);
					}
				break;
			case 3:
				if (value != NULL)
					strncpy (cs42448->mode, value, MAX_STRLEN);
				break;
			case 4:
				if (value != NULL)
					cs42448->format = strtoul (value, NULL, 0);
				break;
			case 5:
				if (value != NULL)
					cs42448->cap_strm_voices = strtoul (value, NULL, 0);
				break;
			case 6:
				idx = 0;
				printf ("CS42448 mixer options:\n");

				/* display all the mixer options before "info" */
				while (idx < opt)
				{
					printf("%8s %s\n", cs42448_opts[idx], opt_help[idx]);
					idx++;
				}
				printf("Example: io-audio -d [driver] mixer=/dev/i2c1:addr=0x48\n");
				break;
			default:
				ado_error ("CS42448: Unrecognized option \"%s\"", value);
				break;
		}
	}
	return (EOK);
}

static  int32_t
cs42448_reset (MIXER_CONTEXT_T * cs42448)
{
	int     cnt;

	/* Power down */
	cs42448_write (cs42448, PWR_CTL, PDN, PDN);		/* Power down Codec */
	cs42448_write (cs42448, PWR_CTL, 0xFE, 0xFE);		/* Power down all DACs and ADCs */

	// SOC is master, codec is slave
	if (strcmp(cs42448->mode, "master") == 0)
		cs42448_write (cs42448, FN_MODE, (ADC_FM_SLAVE | DAC_FM_SLAVE | cs42448->mode_bits), 0xFE);
	
	// SOC is slave, codec is master
	else
		cs42448_write (cs42448, FN_MODE, (ADC_FM_MASTER_SS | DAC_FM_MASTER_SS | cs42448->mode_bits), 0xFE);
	
	/* Configure Interface Formats */
	if (cs42448->format == FORMAT_I2S)
		 cs42448_write (cs42448, IFACE_FMT, (ADC_DIF_I2S | DAC_DIF_I2S), 0xFF);
	else
		cs42448_write (cs42448, IFACE_FMT, (ADC_DIF_LJ | DAC_DIF_LJ), 0xFF);

	/* Configure ADC Control Register */
	/* Based on schematics for i.mx53 Sabre AI board, i.mx6 Q DDR3 boards, both mic inputs are single ended */
	cs42448_write (cs42448, ADC_CTL, (ADC1_SINGLE | ADC2_SINGLE), 0xFF);

	/* Configure Transition Control (Zero Cross & Soft Ramp ) */
	cs42448_write (cs42448, TRANS_CTL,
		DAC_ZCROSS | DAC_SRAMP | ADC_MUTE_SP | ADC_ZCROSS | ADC_SRAMP, 0xFF);

	/* Mute DACs */
	cs42448_write (cs42448, DAC_CH_MUTE, 0xFF, 0xFF);

	/* Initialize DAC Volume to -4.5dB (max volume w/o clipping) */
	for (cnt = 0; cnt < cs42448->play_strm_voices; cnt++)
		cs42448_write (cs42448, AOUT0_VOL_CTL + cnt, 0x09, 0xFF);

	/* Initialize ADC Volume to 0dB */
	for (cnt = 0; cnt < cs42448->cap_strm_voices; cnt++)
		cs42448_write (cs42448, AIN0_VOL_CTL + cnt, 0x00, 0xFF);

	/* Power up */
	cs42448_write (cs42448, PWR_CTL, 0x0, 
	PDN_DAC1 | PDN_DAC2 | PDN_DAC3 | PDN_DAC4 | PDN_ADC1 | PDN_ADC2 | PDN_ADC3);
	cs42448_write (cs42448, PWR_CTL, 0x0, PDN);		/* Power up Codec */

	delay (50);	/* Wait ~2090 FSYNC cycles for codec power up */

	/* Clear mutes */
	cs42448_write (cs42448, DAC_CH_MUTE, 0x0, 0xff);	/* unmute all eight DAC channel */
	cs42448_write (cs42448, TRANS_CTL, 0x0, ADC_MUTE_SP);	/* unmute ADC */

	return 0;
}

static  int32_t
cs42448_destroy (MIXER_CONTEXT_T * cs42448)
{
	ado_debug (DB_LVL_MIXER, "Destroying CS42448 Codec");

	cs42448_write (cs42448, DAC_CH_MUTE, 0xFF, 0xFF);	/* Mute DACs */
	cs42448_write (cs42448, PWR_CTL, PDN, PDN);			/* Power down Codec */
	cs42448_write (cs42448, PWR_CTL, 0xFE, 0xFE);		/* Power down all DACs and ADCs */

	close (cs42448->fd);
	ado_free (cs42448);
	return 0;
}

int
cs42448_mixer (ado_card_t * card, ado_mixer_t ** mixer, char * args, ado_pcm_t * pcm1)
{
	cs42448_context_t *cs42448;
	int32_t status;
	char i2c_dev[_POSIX_PATH_MAX];

	ado_debug (DB_LVL_MIXER, "Initializing CS42448 Codec");

	if ((cs42448 = (cs42448_context_t *) ado_calloc (1, sizeof (cs42448_context_t))) == NULL)
	{
		ado_error ("cs42448: no memory (%s)", strerror (errno));
		return -1;
	}
	if ((status = ado_mixer_create (card, "cs42448", mixer, (void *)cs42448)) != EOK)
	{
		ado_error ("cs42448: Fail to create mixer", strerror (errno));
		ado_free (cs42448);
		return status;
	}

	cs42448->i2c_num = DEFAULT_I2C_NUM;
	cs42448->addr = DEFAULT_I2C_ADDR;
	
	/* This driver currently only support play_strm_voices = 8 */
	cs42448->play_strm_voices = 8;		/* hardcode the play channel to be 7.1 surround */
	cs42448->cap_strm_voices = 4;		/* default  to be 4 voices, override possible via options */
	
	if (cs42448_parse_options (cs42448, args)!=EOK)
	{
		ado_error ("CS42448: Fail to parse mixer options");
		ado_free (cs42448);
		return (-1);
	}

	sprintf(i2c_dev, "/dev/i2c%d", cs42448->i2c_num);
	if ((cs42448->fd = open (i2c_dev, O_RDWR)) < 0)
	{
		ado_error ("cs42448: could not open control device %s", strerror (errno));
		ado_free (cs42448);
		return (-1);
	}

	if (cs42448_mixer_build (cs42448, *mixer))
	{
		ado_error ("CS42448: Failed to build mixer");
		close (cs42448->fd);
		ado_free (cs42448);
		return -1;
	}

	if (cs42448_reset (cs42448))
	{
		close (cs42448->fd);
		ado_free (cs42448);
		return -1;
	}

	ado_mixer_set_reset_func (*mixer, (void *)cs42448_reset);
	ado_mixer_set_destroy_func (*mixer, (void *)cs42448_destroy);

	ado_pcm_chn_mixer (pcm1, ADO_PCM_CHANNEL_PLAYBACK, *mixer,
		ado_mixer_find_element (*mixer, SND_MIXER_ETYPE_PLAYBACK1,
			SND_MIXER_ELEMENT_PLAYBACK, 0), ado_mixer_find_group (*mixer,
			SND_MIXER_PCM_OUT, 0));

	ado_pcm_chn_mixer (pcm1, ADO_PCM_CHANNEL_CAPTURE, *mixer,
		ado_mixer_find_element (*mixer, SND_MIXER_ETYPE_CAPTURE1,
			SND_MIXER_ELEMENT_CAPTURE, 0), ado_mixer_find_group (*mixer, 
				cs42448->cap_strm_voices == 4 ? SND_MIXER_PCM_IN : SND_MIXER_LINE_IN, 0));
				
	if (ado_pcm_sw_mix (card, pcm1, *mixer))
	{
		ado_error ("CS42448: Failed to build software mixer");
		close (cs42448->fd);
		ado_free (cs42448);
		return (-1);
	}

	return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/deva/codecs/cs42448/cs42448.c $ $Rev: 732259 $")
#endif
