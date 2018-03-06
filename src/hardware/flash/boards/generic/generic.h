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
** File: generic.h
**
** Description:
**
** This file contains the definitions for the genric flash library
**
** Ident: $Id: generic.h 680332 2012-11-27 01:28:14Z builder@qnx.com $
*/

/*
** Include Loop Prevention
*/

#ifndef __GENERIC_H_INCLUDED
#define __GENERIC_H_INCLUDED

/*
** Includes
*/

#include <sys/mman.h>
#include <sys/f3s_mtd.h>
#include <sys/neutrino.h>
#include <libc.h>


/*
** Function Prototypes
*/

int32_t generic_open(f3s_socket_t *socket,
                       uint32_t flags);

uint8_t *generic_page(f3s_socket_t *socket,
                        uint32_t page,
                        uint32_t offset,
                        int32_t *size);

int32_t generic_status(f3s_socket_t *socket,
                         uint32_t flags);

void generic_close(f3s_socket_t *socket,
                     uint32_t flags);

#endif /* __GENERIC_H_INCLUDED */

/*
** End
*/

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/flash/boards/generic/generic.h $ $Rev: 680332 $")
#endif
