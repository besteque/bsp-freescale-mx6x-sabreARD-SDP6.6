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



#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <hw/inout.h>
#include <sys/syspage.h>

#include "canmx6x.h"
#include "externs.h"

// Function prototypes
void mdriver_print_data(CANDEV_RINGO_INFO *devinfo);

/*
 * Function to search through the list of mini-drivers for one that matches
 * our interrupt vector.
 *
 * Returns: Matching interrupt vector or -1
 */
int mdriver_init(CANDEV_RINGO_INFO *devinfo, CANDEV_RINGO_INIT *devinit)
{
	struct mdriver_entry	*mdriver = (struct mdriver_entry *)SYSPAGE_ENTRY(mdriver);
	int						num_mdrivers = 0;
	int						mdriver_intr = -1;
	int						mindex;

	// Determine how many mini-drivers are installed
	num_mdrivers = _syspage_ptr->mdriver.entry_size/sizeof(*mdriver);
	#ifdef DEBUG_MDRVR
	fprintf(stderr, "Number of installed mini-drivers = %d\n", num_mdrivers);
	#endif
	// Search through list of mini-drivers for one using our interrupt
	for(mindex = 0; mindex < num_mdrivers; mindex++)
	{
		// Look for a mini-driver matching the system interrupt
		if(mdriver[mindex].intr == devinit->irqsys)
		{
			#ifdef DEBUG_MDRVR
			fprintf(stderr, "Found mdriver %s attached to interrupt %d\n", SYSPAGE_ENTRY(strings)->data + mdriver[mindex].name, mdriver[mindex].intr);
			#endif
			// Found a matching interrupt, stop searching
			mdriver_intr = mdriver[mindex].intr;
			break;
		}
	}

	if(mdriver_intr != -1)
	{
		// Map mini-driver buffer
		if((devinfo->mdata = (minican_data_t *)mmap_device_memory(0, mdriver[mindex].data_size,
								PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, mdriver[mindex].data_paddr)) == NULL)
		{
			perror("Can't map mini-driver data");
			exit(EXIT_FAILURE);
		}
		// Buffered messages of type can_msg_obj_t follow the minican_data_t data structure in memory
		devinfo->mcanmsg = (can_msg_obj_t *)((uint8_t *)devinfo->mdata + sizeof(*devinfo->mdata));
		#ifdef DEBUG_MDRVR
		mdriver_print_data(devinfo);
		#endif
	}
	#ifdef DEBUG_MDRVR
	else
	{
		fprintf(stderr, "No active mini-drivers found on interrupts %d\n", devinit->irqsys);
	}
	#endif

	return(mdriver_intr);
}

/*
 * Function to search though all devices to find a matching message ID
 *
 * Returns: Matching mailbox ID or -1.
 */
int mdriver_find_mbxid(CANDEV_RINGO_INFO *devinfo, uint32_t canmid)
{
	int					i;

	// Search through all receive mailboxes to find a matching message ID
	for(i = devinfo->rxmbxstart; i < (devinfo->numrx + devinfo->rxmbxstart); i++)
	{
		if(canmid == devinfo->canmsg[i].canmid)
		{
			return(i);
		}
	}

	// Didn't find a match
	return(-1);
}

/*
 * Function to add buffered mdriver CAN messages to the driver's message buffer.
 * This function also sorts the messages into the appropriate device according to
 * the message ID.
 *
 * Returns: void
 */
void mdriver_init_data(CANDEV_RINGO_INFO *devinfo, CANDEV_RINGO_INIT *devinit)
{
	minican_data_t		*mdata = devinfo->mdata;
	can_msg_obj_t		*mcanmsg = devinfo->mcanmsg;
	int					i;
	CANDEV_RINGO		*devlist = devinfo->devlist;
	canmsg_t			*rxmsg = NULL;
	int					mbxid = devinfo->rxmbxstart;	/* mailbox index defaults to first rx mailbox */
	uint32_t			*val32;
	short				sort = 0;

	if(!mdata || !mcanmsg)
		return;

	// Determine if data should be sorted based on MID
	if(devinit->flags & INIT_FLAGS_MDRIVER_SORT)
		sort = 1;

	// Loop through all buffered mini-driver messages
	for(i = 0; i < mdata->nrx; i++)
	{
		if(sort)
			// Determine which mailbox this message belongs in
			mbxid = mdriver_find_mbxid(devinfo, mcanmsg[i].canmid);

		if(mbxid != -1)
		{
			// Get the next free message from the free queue
			// No support for overwritting an old message since this
			// is called before we start processing new data
			if((rxmsg = canmsg_dequeue_element(
				devlist[mbxid].cdev.free_queue)) != NULL)
			{
				// Access the data as a uint32_t array
				val32 = (uint32_t *)rxmsg->cmsg.dat;

				// Copy data from mdriver buffer to receive message
				val32[0] = mcanmsg[i].canmdl;
				val32[1] = mcanmsg[i].canmdh;

				// Mini-driver stores raw receive data,
                // determine if we should swap the Endian
				if(devinfo->iflags & INFO_FLAGS_ENDIAN_SWAP)
				{
					// Convert from Big Endian to Little Endian since data is received MSB
					ENDIAN_SWAP32(&val32[0]);
					ENDIAN_SWAP32(&val32[1]);
				}

				// Save off the length of the message
				rxmsg->cmsg.len = (mcanmsg[i].canmcf & MSG_BUF_DLC_MASK) >> MSG_BUF_DLC_SHIFT;

				// Determine if we should store away extra receive message info
				if(devinfo->iflags & INFO_FLAGS_RX_FULL_MSG)
				{
					// Save the message ID
					rxmsg->cmsg.mid = mcanmsg[i].canmid;
					// Save message timestamp (mini-driver doesn't store this info)
					rxmsg->cmsg.ext.timestamp = 0;
				}

				rxmsg->cmsg.ext.is_extended_mid = (mcanmsg[i].canmcf & MB_CNT_IDE) > 0;

				// Add populated element to the receive queue
				canmsg_queue_element(devlist[mbxid].cdev.msg_queue, rxmsg);
				devinfo->stats.received_frames++;
			}
		}
	}
}

/*
 * Function to print out the mdriver's stats and buffered data
 *
 * Returns: void
 */
void mdriver_print_data(CANDEV_RINGO_INFO *devinfo)
{
	minican_data_t		*mdata = devinfo->mdata;
	can_msg_obj_t		*mcanmsg = devinfo->mcanmsg;
	int					i;

	fprintf(stderr, "---------------- MDRIVER DATA -------------------\n");
	fprintf(stderr, "\tMDRIVER_STARTUP         calls = %d\n", mdata->nstartup);
	fprintf(stderr, "\tMDRIVER_STARTUP_PREPARE calls = %d\n", mdata->nstartupp);
	fprintf(stderr, "\tMDRIVER_STARTUP_FINI    calls = %d\n", mdata->nstartupf);
	fprintf(stderr, "\tMDRIVER_KERNEL          calls = %d\n", mdata->nkernel);
	fprintf(stderr, "\tMDRIVER_PROCESS         calls = %d\n", mdata->nprocess);
	fprintf(stderr, "\tNum RX                  calls = %d\n", mdata->nrx);
	fprintf(stderr, "\tTX Enabled              calls = 0x%X\n", mdata->tx_enabled);
	fprintf(stderr, "----------------------------------------------\n");

	fprintf(stderr, "\nBuffered %d CAN messages\n\n", mdata->nrx);
	if(mdata->nrx)
	{
		fprintf(stderr, "MID\t\tMCF\t\tMDH\t\tMDL\n");
		fprintf(stderr, "==========================================================\n");
		for(i = 0; i < mdata->nrx; i++)
		{
			fprintf(stderr, "0x%8X\t0x%8X\t0x%8X\t0x%8X\n",
					mcanmsg[i].canmid, mcanmsg[i].canmcf,
					mcanmsg[i].canmdh, mcanmsg[i].canmdl);
    	}
	}
}




#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/can/mx6x/mdriver.c $ $Rev: 790881 $")
#endif
