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

/*
 * Test program for CAN mini-drivers on the OSK5905 board, when the mini_data handler is used in startup.
 * Connect the HECC1 interfaces of two OSK5905 boards together.  This utility is used to simulate CAN
 * bus traffic.  Boot the OSK5905 board, start the dev-can-osk5905 driver, then run this utility.  Next,
 * boot the second OSK5905 board running a minidriver and verify the data pattern is correct.
 *
 */

#ifdef __USAGE
%C - utility program to generate CAN bus traffic for use with testing mini-drivers

%C - -u #[,rx#][,tx#]  CAN device to operate on (default /dev/can0/tx32)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#define OPT_FLAG_DEVNAME    0x00001 /* Indicate if device name was specified */

int main(int argc, char *argv[])
{
	int                 opt,i;
    char                *cp;
    int                 fd, oflag = O_RDWR;
    char                devname[256];
    unsigned            optflag = 0;
    unsigned            canno = 1;

	char	canmsg[40][9] = {
							"*****01\n\0", "*****02\n\0", "*****03\n\0", "*****04\n\0", "*****05\n\0",
							"*****06\n\0", "*****07\n\0", "*****08\n\0", "*****09\n\0", "*****10\n\0",
							"*****11\n\0", "*****12\n\0", "*****13\n\0", "*****14\n\0", "*****15\n\0",
							"*****16\n\0", "*****17\n\0", "*****18\n\0", "*****19\n\0", "*****20\n\0",
							"*****21\n\0", "*****22\n\0", "*****23\n\0", "*****24\n\0", "*****25\n\0",
							"*****26\n\0", "*****27\n\0", "*****28\n\0", "*****29\n\0", "*****30\n\0",
							"*****31\n\0", "*****32\n\0", "*****33\n\0", "*****34\n\0", "*****35\n\0",
							"*****36\n\0", "*****37\n\0", "*****38\n\0", "*****39\n\0", "*****40\n\0",
							};


    //while (optind < argc) {
    // Process dash options.
    while((opt = getopt(argc, argv, "bdef:Fhm:Mp:Pnrt:Tu:")) != -1)
    {
        switch(opt) {
			case 'u':
            canno = strtoul(optarg, &optarg, 10);
            if((cp = strchr(optarg, ',')))
            {
                sprintf(devname, "/dev/can%u/%s", canno, cp+1);
            }
            else
            {
                sprintf(devname, "/dev/can%u/tx32", canno);
            }
            optflag |= OPT_FLAG_DEVNAME;
            break;
        }
    }
   // No device name args, set default
    if(!(optflag & OPT_FLAG_DEVNAME))
    {
        strcpy(devname, "/dev/can0/tx32");
        optflag |= OPT_FLAG_DEVNAME;
    }

    if( (fd = open(devname, oflag)) == -1 )
    {
        printf("open failed\n");
        exit(EXIT_FAILURE);
    }


	while(1)
	{
		for(i = 0; i < 40; i++)
		{
			//printf("%s", canmsg[i]);
			write(fd, canmsg[i], 8);
		}
	}

	return EXIT_SUCCESS;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/support/mdriver/utils/mdriver-cantest/mdriver-cantest.c $ $Rev: 744940 $")
#endif
