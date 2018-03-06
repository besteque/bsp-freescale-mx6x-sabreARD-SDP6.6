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
** File: genric_page.c
**
** Description:
**
** This file contains the page function for the generic flash library
**
** Ident: $Id: generic_page.c 680332 2012-11-27 01:28:14Z builder@qnx.com $
*/

/*
** Includes
*/

#include "generic.h"


uint8_t *generic_page(f3s_socket_t *socket,
                        uint32_t flags,
                        uint32_t offset,
                        int32_t *size)
{
  if(offset >= socket->window_size)
  {
	errno = ERANGE;
	return NULL;
  }

  /* check if offset does not fit in current page */
  if (!(offset>=socket->window_offset && offset<socket->window_offset+
   socket->window_size))
  {
    /* select proper page */
    socket->window_offset=offset&~(socket->window_size-1);
  }

  /* check if size is wanted */
  if (size)
    *size=min((offset&~(socket->window_size-1))+socket->window_size-offset,
     *size);

  /* return memory pointer */
  return socket->memory+offset;
}

/*
** End
*/

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/flash/boards/generic/generic_page.c $ $Rev: 680332 $")
#endif
