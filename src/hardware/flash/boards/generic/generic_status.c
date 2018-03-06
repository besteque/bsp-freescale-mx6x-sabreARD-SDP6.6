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
** File: generic_status.c
**
** Description:
**
** This file contains the status function for the generic flash library
**
** Ident: $Id: generic_status.c 680332 2012-11-27 01:28:14Z builder@qnx.com $
*/

/*
** Includes
*/

#include "generic.h"

/*
** Function: generic_status
*/

int32_t generic_status(f3s_socket_t *socket,
                         uint32_t flags)
{

  /* everything is fine */

  return EOK;
}

/*
** End
*/

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/flash/boards/generic/generic_status.c $ $Rev: 680332 $")
#endif
