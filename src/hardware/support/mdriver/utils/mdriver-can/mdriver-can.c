/*
 * $QNXLicenseC:
 * Copyright 2014, QNX Software Systems.
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
 * Test program for CAN mini-drivers, when the mini_can handler is used in startup.
 *
 * Opens the minidriver data area and reads the data from it for display.
 *
 * The data header will be printed.  If you want the data then run as: mdriver_can 1
 */

#ifdef __USAGE
%C - test program for CAN mdriver

%C [display]
If display is 1 then the buffered CAN message data will be printed
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <hw/inout.h>
#include <hw/mini_driver.h>

int main(int argc, char *argv[])
{
	int						i;
	int						dump_data = 0;
	uint8_t					*dptr;
	struct minican_data		*mdata;
	int						nrx = 0;
	can_msg_obj_t			*canmsg;
	struct mdriver_entry	*mdriver = (struct mdriver_entry *)SYSPAGE_ENTRY(mdriver);
	int						num_mdrivers = 0;
	int						flag = 0;
	int						mindex;

	if (argv[1])
		dump_data = 1;

	ThreadCtl(_NTO_TCTL_IO, 0);

	// Determine how many mini-drivers are installed
	num_mdrivers = _syspage_ptr->mdriver.entry_size/sizeof(*mdriver);
	//fprintf(stderr, "Number of installed mini-drivers = %d\n", num_mdrivers);

	// Search through list of mini-drivers for one using our interrupt
	for(mindex = 0; mindex < num_mdrivers; mindex++)
	{
		if(strcmp(MINI_CAN_NAME, SYSPAGE_ENTRY(strings)->data + mdriver[mindex].name) == 0)
		{
			//fprintf(stderr, "Found mdriver %s attached to interrupt %d\n", SYSPAGE_ENTRY(strings)->data + mdriver[mindex].name, mdriver[mindex].intr);
			flag = 1;
			break;
		}
	}

	if(flag == 0)
	{
		fprintf(stderr, "No matching mini-drivers found\n");
		return EXIT_SUCCESS;
	}

	/* map in minidriver data area */
	if ((dptr = mmap_device_memory(0, 65536, PROT_READ | PROT_WRITE | PROT_NOCACHE, 0,
			SYSPAGE_ENTRY(mdriver)->data_paddr)) == NULL)
	{
		fprintf(stderr, "Unable to get data pointer\n");
		return (-1);
	}

	mdata = (struct minican_data *) dptr;
	dptr = dptr + sizeof(struct minican_data);
	canmsg = (can_msg_obj_t *)dptr;

	/* dump mini-driver data */
	fprintf(stderr, "---------------- MDRIVER DATA -------------------\n");
	fprintf(stderr, "\tMDRIVER_STARTUP         calls = %d\n", mdata->nstartup);
	fprintf(stderr, "\tMDRIVER_STARTUP_PREPARE calls = %d\n", mdata->nstartupp);
	fprintf(stderr, "\tMDRIVER_STARTUP_FINI    calls = %d\n", mdata->nstartupf);
	fprintf(stderr, "\tMDRIVER_KERNEL          calls = %d\n", mdata->nkernel);
	fprintf(stderr, "\tMDRIVER_PROCESS         calls = %d\n", mdata->nprocess);
	fprintf(stderr, "\tNum RX                  calls = %d\n", mdata->nrx);
	fprintf(stderr, "\tTX Enabled              calls = 0x%X\n", mdata->tx_enabled);
	nrx = mdata->nrx;
	fprintf(stderr, "----------------------------------------------\n");

	fprintf(stderr, "\nBuffered %d CAN messages\n\n", nrx);
	if(dump_data && nrx)
	{
		fprintf(stderr, "MID\t\tMCF\t\tMDH\t\tMDL\n");
		fprintf(stderr, "==========================================================\n");
		for(i = 0; i < nrx; i++)
		{
			fprintf(stderr, "0x%8X\t0x%8X\t0x%8X\t0x%8X\n",
			canmsg[i].canmid, canmsg[i].canmcf,
			canmsg[i].canmdh, canmsg[i].canmdl);
		}
	}

	return EXIT_SUCCESS;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/support/mdriver/utils/mdriver-can/mdriver-can.c $ $Rev: 776821 $")
#endif
