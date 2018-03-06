/*
 * $QNXLicenseC:
 * Copyright 2014, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable license fees to QNX
 * Software Systems before you may reproduce, modify or distribute this software,
 * or any work that includes all or part of this software.   Free development
 * licenses are available for evaluation and non-commercial purposes.  For more
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *
 * This file may contain contributions from others.  Please review this entire
 * file for other proprietary rights or license notices, as well as the QNX
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/
 * for other information.
 * $
 */

#include "startup.h"
#include <hw/mini_driver.h>
#include <arm/mx6x.h>

#define MDRIVE_DEBUG
#define MDRIVE_DEBUG_DATA

/**********************************************************************
 * Function to modify individual register bits.
 *
 * Define as a macro so it can be called safely once we enter the
 * kernel environment. Function prototype:
 *
 * void SET_PORT32(unsigned port, uint32_t mask, uint32_t data)
 **********************************************************************/
#define SET_PORT32(port, mask, data)					\
{														\
	out32(port, (in32(port) & ~mask) | (data & mask));	\
}

#define INT_ENABLE(port)										\
{																\
	SET_PORT32(port, MX6X_EPIT_CR_OCIEN, MX6X_EPIT_CR_OCIEN);	\
}

#define INT_DISABLE(port)										\
{																\
	SET_PORT32(port, MX6X_EPIT_CR_OCIEN, ~MX6X_EPIT_CR_OCIEN);	\
}


/**********************************************************************
 * Function to initialize the timer.
 *
 * Don't need to define as a macro since this is only called during Startup.
 *
 * int init_hw(struct minitimet_data *mdata)
 **********************************************************************/
static int init_hw( minitimer_data_t *mdata)
{
	/* Map register access to device for Startup environment */
	if((mdata->canport = startup_io_map(MX6X_EPIT_SIZE, MX6X_EPIT2_BASE)) == 0) return(-1);

	out32(mdata->canport + MX6X_EPIT_CR, 0);//disable the timer
    out32(mdata->canport + MX6X_EPIT_SR, 1);//clear int status

    SET_PORT32(mdata->canport + MX6X_EPIT_CR, MX6X_EPIT_CR_CLKSRC(SET_MX6X_EPIT_CLKSRC_PCLK), MX6X_EPIT_CR_CLKSRC(SET_MX6X_EPIT_CLKSRC_PCLK));
    SET_PORT32(mdata->canport + MX6X_EPIT_CR, MX6X_EPIT_CR_RLD, MX6X_EPIT_CR_RLD);		//set and forget mode
    SET_PORT32(mdata->canport + MX6X_EPIT_CR, MX6X_EPIT_CR_IOVW, MX6X_EPIT_CR_IOVW);	//enable update counter
    SET_PORT32(mdata->canport + MX6X_EPIT_CR, MX6X_EPIT_CR_OCIEN, MX6X_EPIT_CR_OCIEN);	//enable compare
    SET_PORT32(mdata->canport + MX6X_EPIT_CR, MX6X_EPIT_CR_EN, MX6X_EPIT_CR_EN);

    out32(mdata->canport + MX6X_EPIT_LR, SET_MX6X_EPIT_LR);//load value

#ifdef MDRIVE_DEBUG
	kprintf("mini-driver init_hw: mdata->canport 0x%x\n", mdata->canport);
	kprintf("MX6X_EPIT_CR 0x%x\n", in32(mdata->canport + MX6X_EPIT_CR));
	kprintf("MX6X_EPIT_SR 0x%x\n", in32(mdata->canport + MX6X_EPIT_SR));
	kprintf("MX6X_EPIT_LR 0x%x\n", in32(mdata->canport + MX6X_EPIT_LR));
	kprintf("MX6X_EPIT_CMPR 0x%x\n", in32(mdata->canport + MX6X_EPIT_CMPR));
	kprintf("MX6X_EPIT_CNR 0x%x\n", in32(mdata->canport + MX6X_EPIT_CNR));
#endif

	return(0);
}

/**********************************************************************
 * Minidriver Handler Function.
 *
 * int mini_timer(int state, void *data)
 **********************************************************************/
int mini_timer(int state, void *data)
{
	uint32_t			*dptr;
	minitimer_data_t	*mdata;

	// Cast the data pointer to our mini-driver data structure
	mdata = (struct minitimer_data *)data;
	dptr = (uint32_t *) (mdata + 1);

	// MDRIVER_INTR_ATTACH - Full driver has attached to the interrupt
	if(state == MDRIVER_INTR_ATTACH)
	{
		// Must disable all interrupts
		INT_DISABLE(mdata->canport + MX6X_EPIT_CR);
		return(1); // Minidriver is done and won't be called again
	}
	// MDRIVER_INIT - Initialize the hardware and setup the data area
	else if(state == MDRIVER_INIT)
	{
		#ifdef MDRIVE_DEBUG
		kprintf("MDRIVER_INIT\n");
		#endif

		// Set the Startup poll rate
		mdriver_max = KILO(MINI_DATA_STARTUP_DEFAULT_RATE);

		if(init_hw(mdata) == -1)
		{
			// There was an error, disable the mini-driver
			#ifdef MDRIVE_DEBUG
			kprintf("MDRIVER_INIT init_hw() FAILED\n");
			#endif

			return(1); // Minidriver is done and won't be called again
		}
		// Clear the statistics counters
		mdata->nstartup = 0;
		mdata->nstartupf = 0;
		mdata->nstartupp = 0;
		mdata->nkernel = 0;
		mdata->nprocess = 0;
		mdata->data_len = 0;

		return(0); // Minidriver will be called again
	}
	// MDRIVER_STARTUP_PREPARE - Prepare mini-driver for switch from Startup to Kernel environment
	else if(state == MDRIVER_STARTUP_PREPARE)
	{
		#ifdef MDRIVE_DEBUG
		kprintf("MDRIVER_STARTUP_PREPARE\n");
		#endif

		mdata->nstartupp++;

		// Map register access to the device for Kernel environment
		if((mdata->canport_k = callout_io_map(MX6X_EPIT_SIZE, MX6X_EPIT2_BASE)) == 0)
		{
			// There was an error, disable the mini-driver
			#ifdef MDRIVE_DEBUG
			kprintf("MDRIVER_STARTUP_PREPARE callout_io_map FAILED\n");
			#endif
			// Disable all interrupts
			INT_DISABLE(mdata->canport + MX6X_EPIT_CR);
			return(1); // Minidriver is done and won't be called again
		}
	}
	// MDRIVER_STARTUP_STARTUP - mini-driver called during Startup
	else if(state == MDRIVER_STARTUP)
	{
		mdata->nstartup++;
		#ifdef MDRIVE_DEBUG
		/* Normally, don't call this. It's too much and might cause system crash */
		//kprintf("MDRIVER_STARTUP\n"); //don't call this. it's too much and cause sys hanging
		#endif
	}
	else if(state == MDRIVER_KERNEL)
	{
		mdata->nkernel++;
		// Can't call kprintf() once we are called by the kernel
	}
	else if(state == MDRIVER_PROCESS)
	{
		mdata->nprocess++;
		// Can't call kprintf() once processes are running
	}

	/* Check interrupt
	 * If we are in Startup, we are not actually called by an interrupt
	 * (interrupts are globally disabled) but these bits are still set by the device.
	 */
	if(in32(mdata->canport +  MX6X_EPIT_SR)) out32(mdata->canport +  MX6X_EPIT_SR, 1);

    /* put state and timer counter in the data area */
    if (mdata->data_len < (MINI_DATA_BUF_SIZE /4 - sizeof(minitimer_data_t))) {
        dptr[mdata->data_len] = (in32(mdata->canport + MX6X_EPIT_CNR) & MINI_TIMER_COUNTER_NASK) | (state << MINI_TIMER_STATE_SHIFT);
        mdata->data_len++;
    }

	// MDRIVER_STARTUP_FINI - Next time mini-driver is called will be in Kernel environment
	if(state == MDRIVER_STARTUP_FINI)
	{
		mdata->nstartupf++;

		#ifdef MDRIVE_DEBUG
		kprintf("MDRIVER_STARTUP_FINI\n");
		#endif

		/* Switch register and memory mappings from Startup to Kernel environment */
		mdata->canport = mdata->canport_k;
	}

	return(0); // Minidriver will be called again
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/sabreARD/mini_timer.c $ $Rev: 745085 $")
#endif
