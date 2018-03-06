/*
 * $QNXLicenseC: 
 * Copyright 2008, QNX Software Systems.  
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
#include "mxesai.h"

extern int cs42448_mixer (ado_card_t * card, ado_mixer_t ** mixer, char * args, ado_pcm_t * pcm1);

/******************************
 * Called by audio controller *
 *****************************/
int
codec_mixer (ado_card_t * card, HW_CONTEXT_T * mxesai, ado_pcm_t * pcm)
{
    return (cs42448_mixer (card, &(mxesai->mixer), mxesai->mixeropts, pcm));
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/deva/ctrl/mxesai/nto/arm/dll.le.v7.mx6sabreARD/mx6ard.c $ $Rev: 680793 $")
#endif
