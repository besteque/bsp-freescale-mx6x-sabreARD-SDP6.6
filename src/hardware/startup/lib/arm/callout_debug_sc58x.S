/*
 * $QNXLicenseC:
 * Copyright 2016, QNX Software Systems.
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
 * polled serial I/O
 */

#include "callout.ah"
#include <arm/sc58x.h>

/*
 * -----------------------------------------------------------------------
 * Routine to patch callout code
 *
 * On entry:
 *	r0	- physical address of syspage
 *	r1	- virtual  address of syspage
 *	r2	- offset from start of syspage to start of the callout routine
 *	r3	- offset from start of syspage to read/write data used by callout
 *	[sp]	- data registered with callout via callout_register_data()
 *	[sp+4]	- address of callout definition (CALLOUT_START)
 * -----------------------------------------------------------------------
 */
patch_debug:
    stmdb   sp!, {r4, lr}
    add     r4, r0, r2          // address of callout routine

    // Map registers
    ldr     r1, [sp, #8]        // debug_device[...]
    ldr     r1, [r1, #DDI_BASE]

    ldr     r0, Lpaddr
    cmp     r1, r0
    strne   r1, Lpaddr
    ldreq   r0, Lvaddr
    beq     1f

    mov     r0, #0x100
    bl      callout_io_map32
    str     r0, Lvaddr

    /* Patch the callout routine */
1:  CALLOUT_PATCH   r4, r0, r1, r2, ip
    ldmia   sp!, {r4, pc}

Lpaddr: .word   -1
Lvaddr: .word   -1

/*
 * -----------------------------------------------------------------------
 * void	display_char_sc58x(struct sypage_entry *, char)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(display_char_sc58x, 0, patch_debug)
    mov     ip, #0x000000ff         // register base (= tx holding register)
    orr     ip, ip, #0x0000ff00
    orr     ip, ip, #0x00ff0000
    orr     ip, ip, #0xff000000

loop1:
    ldr     r3, [ip, #SC58X_UART_STAT]
    tst     r3, #UART_STAT_THRE
    beq     loop1

    str     r1, [ip, #SC58X_UART_THR]

    mov     pc, lr
CALLOUT_END(display_char_sc58x)

/*
 * -----------------------------------------------------------------------
 * char	poll_key_sc58x(struct sypage_entry *)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(poll_key_sc58x, 0, patch_debug)
    mov     ip, #0x000000ff
    orr     ip, ip, #0x0000ff00
    orr     ip, ip, #0x00ff0000
    orr     ip, ip, #0xff000000

    ldr     r3, [ip, #SC58X_UART_STAT]
    tst     r3, #UART_STAT_DR
    ldrne   r0, [ip, #SC58X_UART_RBR]
    mvneq   r0, #1

    mov     pc, lr
CALLOUT_END(poll_key_sc58x)

/*
 * -----------------------------------------------------------------------
 * void	break_detect_sc58x(struct sypage_entry *, char)
 * -----------------------------------------------------------------------
 */
CALLOUT_START(break_detect_sc58x, 0, patch_debug)
    mov     ip, #0x000000ff
    orr     ip, ip, #0x0000ff00
    orr     ip, ip, #0x00ff0000
    orr     ip, ip, #0xff000000

    ldr     r3, [ip, #SC58X_UART_STAT]
    tst     r3, #UART_STAT_BI
    moveq   r0, #UART_STAT_BI
    streq   r0, [ip, #SC58X_UART_STAT]

    mov     pc, lr
CALLOUT_END(break_detect_sc58x)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/lib/arm/callout_debug_sc58x.S $ $Rev: 801974 $";
.previous
#endif
#endif
