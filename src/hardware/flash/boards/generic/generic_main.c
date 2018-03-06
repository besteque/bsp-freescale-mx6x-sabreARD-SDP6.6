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


/*
** File: genreic_main.c
**
** Description:
**
** This file contains the main function for the generic f3s flash file system
**
** Ident: $Id: generic_main.c 809703 2016-08-03 15:29:20Z nihamilton@qnx.com $
**
*/

/*
** Includes
*/
#include "generic.h"

/*
** Function: main
*/

int main(int argc, char **argv)
{
	static f3s_service_t service[] =
	{
		{
			sizeof(f3s_service_t),
			generic_open,
			generic_page,
			generic_status,
			generic_close
		},
		{
			0, 0, 0, 0, 0  /* mandatory last entry */
		}
	};

#if MTD_VER == 2
	static f3s_flash_v2_t flash[] =
	{
		{
			sizeof(f3s_flash_v2_t),
			f3s_i28f008_ident,		/* Common Ident             */
			f3s_i28f008_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_i28f008_v2write,	/* v2 Write                 */
			f3s_iCFI_v2erase,		/* v2 Erase                 */
			f3s_iCFI_v2suspend,		/* v2 Suspend               */
			f3s_iCFI_v2resume,		/* v2 Resume                */
			f3s_iCFI_v2sync,		/* v2 Sync                  */
			/* v2 Islock/Lock/Unlock/Unlockall (not supported)  */
			NULL, NULL, NULL, NULL
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_i28f800_ident,		/* Common Ident             */
			f3s_i28f008_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_i28f008_v2write,	/* v2 Write                 */
			f3s_iCFI_v2erase,		/* v2 Erase                 */
			f3s_iCFI_v2suspend,		/* v2 Suspend               */
			f3s_iCFI_v2resume,		/* v2 Resume                */
			f3s_iCFI_v2sync,		/* v2 Sync                  */
			/* v2 Islock/Lock/Unlock/Unlockall (not supported)  */
			NULL, NULL, NULL, NULL
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_nuCFI_ident,		/* Common Ident             */
			f3s_a29f040_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_aCFI_v2write,		/* v2 Write                 */
			f3s_a29f100_v2erase,	/* v2 Erase                 */
			f3s_nuCFI_v2suspend,	/* v2 Suspend               */
			f3s_a29f040_v2resume,	/* v2 Resume                */
			f3s_nuCFI_v2sync,	/* v2 Sync                  */
			f3s_aCFI_v2islock,
			f3s_aCFI_v2lock,
			f3s_aCFI_v2unlock,
			f3s_aCFI_v2unlockall,
			f3s_aMB_v2ssrop
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_iCFI_ident,			/* Common Ident             */
			f3s_i28f008_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_iCFI_v2write,		/* v2 Write                 */
			f3s_iCFI_v2erase,		/* v2 Erase                 */
			f3s_iCFI_v2suspend,		/* v2 Suspend               */
			f3s_iCFI_v2resume,		/* v2 Resume                */
			f3s_iCFI_v2sync,		/* v2 Sync                  */
			f3s_iCFI_v2islock,		/* v2 Islock                */
			f3s_iCFI_v2lock,		/* v2 Lock                  */
			f3s_iCFI_v2unlock,		/* v2 Unlock                */
			f3s_iCFI_v2unlockall	/* v2 Unlockall             */
		}, 
		{
			sizeof(f3s_flash_v2_t),
			f3s_s29glxxxs_ident,		/* Common Ident             */
			f3s_s29glxxxs_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			f3s_s29glxxxs_v2read,		/* v2 Read                  */
			f3s_s29glxxxs_v2write,		/* v2 Write                 */
			f3s_s29glxxxs_v2erase,		/* v2 Erase                 */
			f3s_s29glxxxs_v2suspend,	/* v2 Suspend               */
			f3s_s29glxxxs_v2resume,		/* v2 Resume                */
			f3s_s29glxxxs_v2sync,		/* v2 Sync                  */
			f3s_s29glxxxs_v2islock,
			f3s_s29glxxxs_v2lock,
			f3s_s29glxxxs_v2unlock,
			f3s_s29glxxxs_v2unlockall,
			f3s_s29glxxxs_v2ssrop
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_aCFI_ident,			/* Common Ident             */
			f3s_a29f040_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_aCFI_v2write,		/* v2 Write                 */
			f3s_a29f100_v2erase,	/* v2 Erase                 */
			f3s_a29f040_v2suspend,	/* v2 Suspend               */
			f3s_a29f040_v2resume,	/* v2 Resume                */
			f3s_a29f040_v2sync,		/* v2 Sync                  */
			f3s_aCFI_v2islock,
			f3s_aCFI_v2lock,
			f3s_aCFI_v2unlock,
			f3s_aCFI_v2unlockall,
			f3s_aMB_v2ssrop
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_aMB_ident,			/* Common Ident             */
			f3s_a29f040_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_aCFI_v2write,		/* v2 Write                 */
			f3s_aMB_v2erase,		/* v2 Erase                 */
			f3s_aCFI_v2suspend,		/* v2 Suspend               */
			f3s_aMB_v2resume,		/* v2 Resume                */
			f3s_aMB_v2sync,			/* v2 Sync                  */
			/* v2 Islock/Lock/Unlock/Unlockall (not supported)  */
			NULL, NULL, NULL, NULL
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_a29f040_ident,		/* Common Ident             */
			f3s_a29f040_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_a29f040_v2write,	/* v2 Write                 */
			f3s_a29f040_v2erase,	/* v2 Erase                 */
			f3s_a29f040_v2suspend,	/* v2 Suspend               */
			f3s_a29f040_v2resume,	/* v2 Resume                */
			f3s_a29f040_v2sync,		/* v2 Sync                  */
			/* v2 Islock/Lock/Unlock/Unlockall (not supported)  */
			NULL, NULL, NULL, NULL
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_a29f004_ident,		/* Common Ident             */
			f3s_a29f040_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_a29f040_v2write,	/* v2 Write                 */
			f3s_a29f040_v2erase,	/* v2 Erase                 */
			f3s_a29f040_v2suspend,	/* v2 Suspend               */
			f3s_a29f040_v2resume,	/* v2 Resume                */
			f3s_a29f040_v2sync,		/* v2 Sync                  */
			/* v2 Islock/Lock/Unlock/Unlockall (not supported)  */
			NULL, NULL, NULL, NULL
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_a29f100_ident,		/* Common Ident             */
			f3s_a29f040_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_a29f100_v2write,	/* v2 Write                 */
			f3s_a29f100_v2erase,	/* v2 Erase                 */
			f3s_a29f040_v2suspend,	/* v2 Suspend               */
			f3s_a29f040_v2resume,	/* v2 Resume                */
			f3s_a29f040_v2sync,		/* v2 Sync                  */
			/* v2 Islock/Lock/Unlock/Unlockall (not supported)  */
			NULL, NULL, NULL, NULL
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_aCFIsm_ident,		/* Comman SM Ident          */
			f3s_a29f040_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_a29f100_v2write,	/* v2 Write                 */
			f3s_a29f100_v2erase,	/* v2 Erase                 */
			f3s_a29f040_v2suspend,	/* v2 Suspend               */
			f3s_a29f040_v2resume,	/* v2 Resume                */
			f3s_a29f040_v2sync,		/* v2 Sync                  */
			/* v2 Islock/Lock/Unlock/Unlockall (not supported)  */
			NULL, NULL, NULL, NULL
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_s28f008_ident,		/* Common Ident             */
			f3s_i28f008_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_i28f008_v2write,	/* v2 Write                 */
			f3s_iCFI_v2erase,		/* v2 Erase                 */
			f3s_iCFI_v2suspend,		/* v2 Suspend               */
			f3s_iCFI_v2resume,		/* v2 Resume                */
			f3s_iCFI_v2sync,		/* v2 Sync                  */
			/* v2 Islock/Lock/Unlock/Unlockall (not supported)  */
			NULL, NULL, NULL, NULL
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_f29f040_ident,		/* Common Ident             */
			f3s_a29f040_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_a29f040_v2write,	/* v2 Write                 */
			f3s_a29f040_v2erase,	/* v2 Erase                 */
			f3s_a29f040_v2suspend,	/* v2 Suspend               */
			f3s_a29f040_v2resume,	/* v2 Resume                */
			f3s_a29f040_v2sync,		/* v2 Sync                  */
			/* v2 Islock/Lock/Unlock/Unlockall (not supported)  */
			NULL, NULL, NULL, NULL
		},
		{
			sizeof(f3s_flash_v2_t),
			f3s_hyCFI_ident,		/* Common Ident             */
			f3s_a29f040_reset,		/* Common Reset             */

			/* v1 ReadWrite/Erase/Suspend/Resume/Sync (Unused)  */
			NULL, NULL, NULL, NULL, NULL, NULL,

			NULL,					/* v2 Read    (Use default) */
			f3s_aCFI_v2write,		/* v2 Write                 */
			f3s_a29f100_v2erase,	/* v2 Erase                 */
			f3s_a29f040_v2suspend,	/* v2 Suspend               */
			f3s_a29f040_v2resume,	/* v2 Resume                */
			f3s_a29f040_v2sync,		/* v2 Sync                  */
			f3s_aCFI_v2islock,
			f3s_aCFI_v2lock,
			f3s_aCFI_v2unlock,
			f3s_aCFI_v2unlockall,
			f3s_aMB_v2ssrop
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0	/* mandatory last entry */
		}
	};
#else
	static f3s_flash_t flash[] =
	{
		{
			sizeof(f3s_flash_t),
			f3s_i28f008_ident,
			f3s_i28f008_reset,
			NULL,
			f3s_i28f008_write,
			f3s_i28f008_erase,
			f3s_i28f008_suspend,
			f3s_i28f008_resume,
			f3s_i28f008_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_i28f800_ident,
			f3s_i28f008_reset,
			NULL,
			f3s_i28f008_write,
			f3s_i28f008_erase,
			f3s_i28f008_suspend,
			f3s_i28f008_resume,
			f3s_i28f800_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_iCFI_ident,
			f3s_i28f008_reset,
			NULL,
			f3s_iCFI_write,
			f3s_i28f008_erase,
			f3s_i28f008_suspend,
			f3s_i28f008_resume,
			f3s_i28f008_sync
		},   
		{
			sizeof(f3s_flash_t),
			f3s_aMB_ident,
			f3s_a29f040_reset,
			NULL,
			f3s_aCFI_write,
			f3s_aMB_erase,
			f3s_aCFI_suspend,
			f3s_aMB_resume,
			f3s_aMB_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_aCFI_ident,
			f3s_a29f040_reset,
			NULL,
			f3s_aCFI_write,
			f3s_a29f100_erase,
			f3s_a29f040_suspend,
			f3s_a29f040_resume,
			f3s_a29f004_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_a29f040_ident,
			f3s_a29f040_reset,
			NULL,
			f3s_a29f040_write,
			f3s_a29f040_erase,
			f3s_a29f040_suspend,
			f3s_a29f040_resume,
			f3s_a29f040_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_a29f004_ident,
			f3s_a29f040_reset,
			NULL,
			f3s_a29f040_write,
			f3s_a29f040_erase,
			f3s_a29f040_suspend,
			f3s_a29f040_resume,
			f3s_a29f004_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_a29f100_ident,
			f3s_a29f040_reset,
			NULL,
			f3s_a29f100_write,
			f3s_a29f100_erase,
			f3s_a29f040_suspend,
			f3s_a29f040_resume,
			f3s_a29f004_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_aCFIsm_ident,
			f3s_a29f040_reset,
			NULL,
			f3s_a29f100_write,
			f3s_a29f100_erase,
			f3s_a29f040_suspend,
			f3s_a29f040_resume,
			f3s_a29f004_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_s28f008_ident,
			f3s_i28f008_reset,
			NULL,
			f3s_i28f008_write,
			f3s_i28f008_erase,
			f3s_i28f008_suspend,
			f3s_i28f008_resume,
			f3s_i28f008_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_f29f040_ident,
			f3s_a29f040_reset,
			NULL,
			f3s_a29f040_write,
			f3s_a29f040_erase,
			f3s_a29f040_suspend,
			f3s_a29f040_resume,
			f3s_a29f040_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_hyCFI_ident,
			f3s_a29f040_reset,
			NULL,
			f3s_aCFI_write,
			f3s_a29f100_erase,
			f3s_a29f040_suspend,
			f3s_a29f040_resume,
			f3s_a29f004_sync
		},
		{
			sizeof(f3s_flash_t),
			f3s_nuCFI_ident,
			f3s_a29f040_reset,
			NULL,
			f3s_aCFI_write,
			f3s_a29f100_erase,
			f3s_nuCFI_v2suspend,
			f3s_a29f040_resume,
			f3s_a29f004_sync
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0  /* mandatory last entry */
		}
	};
#endif

	/* init f3s */
	f3s_init(argc, argv, (f3s_flash_t *)flash);

	/* start f3s */
	return f3s_start(service, (f3s_flash_t *)flash);
}

/*
** End
*/

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/flash/boards/generic/generic_main.c $ $Rev: 809703 $")
#endif
