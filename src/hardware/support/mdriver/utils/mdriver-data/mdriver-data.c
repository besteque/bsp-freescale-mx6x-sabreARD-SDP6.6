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
 * Test program for minidrivers, when the mini_data handler is used in startup.
 *
 * Opens the minidriver data area and reads the data from it for display.
 *
 * The data header will be printed.  If you want the data then run as mdrvr-data 1
 */

#ifdef __USAGE
%C - test program for minidrivers, when startup is built with the mini_data handler

%C [display]
If display is 1 then the data area of the minidriver will be printed showing
state information for minidriver calls
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <hw/inout.h>
#include <hw/mini_driver.h>

int main(int argc, char *argv[])
{
	int			i,	count;
	int			dump_data = 0;
	uint8_t		*dptr;
	mini_data_t	*mdata;
	struct mdriver_entry	*mdriver = (struct mdriver_entry *)SYSPAGE_ENTRY(mdriver);
	int	num_mdrivers = 0, flag =0, mindex;

	if (argv[1]) dump_data = 1;

	ThreadCtl(_NTO_TCTL_IO_PRIV, 0);

   /* Determine how many mini-drivers are installed */
    num_mdrivers = _syspage_ptr->mdriver.entry_size/sizeof(*mdriver);

    /* Search through list of mini-drivers for one using our interrupt */
    for(mindex = 0; mindex < num_mdrivers; mindex++)
    {
        if(strcmp(MINI_DATA_NAME, SYSPAGE_ENTRY(strings)->data + mdriver[mindex].name) == 0)
        {
            fprintf(stderr, "Found mdriver %s attached to interrupt %d\n", SYSPAGE_ENTRY(strings)->data + mdriver[mindex].name, mdriver[mindex].intr);
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
	if ((dptr = mmap_device_memory(0,  MINI_DATA_BUF_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE, 0,
			SYSPAGE_ENTRY(mdriver)->data_paddr)) == NULL)
	{
		fprintf(stderr, "Unable to get data pointer\n");
		return (-1);
	}

	mdata = (mini_data_t *) dptr;
	dptr = dptr + sizeof(mini_data_t);

	/* dump mini-driver data */
	printf("---------------- MDRIVER DATA -------------------\n");
	printf("state  state name               calls\n");
    printf("  %d    MDRIVER_STARTUP          %d\n", MDRIVER_STARTUP, mdata->nstartup);
    printf("  %d    MDRIVER_STARTUP_PREPARE  %d\n", MDRIVER_STARTUP_PREPARE, mdata->nstartupp);
    printf("  %d    MDRIVER_STARTUP_FINI     %d\n", MDRIVER_STARTUP_FINI, mdata->nstartupf);
    printf("  %d    MDRIVER_KERNEL           %d\n", MDRIVER_KERNEL, mdata->nkernel);
    printf("  %d    MDRIVER_PROCESS          %d\n", MDRIVER_PROCESS, mdata->nprocess);
    printf("       Total calls              %d\n", mdata->data_len);

	count = mdata->data_len;

	if (dump_data)
	{
		printf("\nState information:\n");
		for (i = 0; i < count; i++){
			if(0 == i%16) printf("\n(%04d): ", i);
			printf("%d  ", dptr[i]);

		}
	}
	printf("\n---------------------------------\n");

	return EXIT_SUCCESS;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/support/mdriver/utils/mdriver-data/mdriver-data.c $ $Rev: 745387 $")
#endif
