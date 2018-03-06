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

#include "callout.ah"
#include <arm/sc58x.h>

/*
 * --------------------------------------------------------------------------
 * Routine to patch callout code
 *
 * On entry:
 *	r0 - physical address of syspage
 *	r1 - virtual  address of syspage
 *	r2 - offset from start of syspage to start of the callout routine
 *	r3 - offset from start of syspage to read/write data used by callout
 * --------------------------------------------------------------------------
 */
patch_timer:
    stmdb   sp!,{r4,lr}
    add     r4, r0, r2              // address of callout routine

    /*
     * Already mapped in?
     */
    ldr     r0, Lvaddr
    cmp     r0, #-1
    bne     1f

    /*
     * Map registers
     */
    mov	    r0, #0x800              // size of registers
    ldr     r1, Lpaddr
    bl      callout_io_map32
    str     r0, Lvaddr

    /*
     * Patch the callout routine
     */
1:  CALLOUT_PATCH   r4, r0, r1, r2, ip

    ldmia	sp!,{r4,pc}

Lpaddr:	.word	SC58X_TIMER0_BASE
Lvaddr: .word   -1

/*
 * --------------------------------------------------------------------------
 * Set clock resolution, and enable interrupt triggering as a side effect.
 * The interrupt remains masked until it is enabled via intr_unmask_omap
 *
 * On entry:
 *	r0 - pointer to syspage_entry
 *	r1 - pointer to qtime_entry
 * --------------------------------------------------------------------------
 */
CALLOUT_START(timer_load_sc58x, 0, patch_timer)
    /*
     * Get the address of the timer registers (patched)
     */
    mov     ip,     #0x000000ff
    orr     ip, ip, #0x0000ff00
    orr     ip, ip, #0x00ff0000
    orr     ip, ip, #0xff000000

    /*
     * Get the load value
     */
    ldr     r0, [r1, #QT_TIMER_LOAD]

    /*
     * Stop the timer
     */
    mov     r1, #1
    strh    r1, [ip, #SC58X_TIMER0_STOP_SET]
    strh    r1, [ip, #SC58X_TIMER0_RUN_CLR]

    /*
     * load counter value
     */
    str     r0, [ip, #SC58X_TIMER0_TMR_PER(0)]
    add     r0, r0, #-1
    str     r0, [ip, #SC58X_TIMER0_TMR_WID(0)]

    /*
     *  start the timer running
     */
    strh    r1, [ip, #SC58X_TIMER0_RUN_SET]

    mov     pc, lr
CALLOUT_END(timer_load_sc58x)


/*
 * --------------------------------------------------------------------------
 * Read the current timer value, relative to the last clock tick
 *
 * On entry:
 *	r0 - pointer to syspage_entry
 *	r1 - pointer to qtime_entry
 *
 * Note: this doesn't deal with the counter wrapping, eg. ClockCycles just
 *		  at the point where the clock interrupt is triggerred.
 * --------------------------------------------------------------------------
 */
CALLOUT_START(timer_value_sc58x, 0, patch_timer)
    /*
    * Get the address of the timer registers (patched)
    */
    mov     ip,     #0x000000ff
    orr     ip, ip, #0x0000ff00
    orr     ip, ip, #0x00ff0000
    orr     ip, ip, #0xff000000

    /*
     * Latch the current timer value and return qtp->timer_load - value
     */
    ldr     r0, [ip, #SC58X_TIMER0_TMR_CNT(0)]

    mov     pc, lr
CALLOUT_END(timer_value_sc58x)


/*
 * --------------------------------------------------------------------------
 * Clear timer interrupt.
 *
 * On entry:
 *	r0 - pointer to syspage_entry
 *	r1 - pointer to qtime_entry
 * --------------------------------------------------------------------------
 */
CALLOUT_START(timer_reload_sc58x, 0, patch_timer)
    /*
     * Get the address of the timer registers (patched)
     */
    mov     ip,     #0x000000ff
    orr     ip, ip, #0x0000ff00
    orr     ip, ip, #0x00ff0000
    orr     ip, ip, #0xff000000

    mov     r0, #1
    strh    r0, [ip, #SC58X_TIMER0_DATA_ILAT]

    mov     pc, lr
CALLOUT_END(timer_reload_sc58x)

#ifdef __QNXNTO__
#ifdef __USESRCVERSION
.section .ident, "SM",%progbits,1;
.asciz "$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/lib/arm/callout_timer_sc58x.S $ $Rev: 801974 $";
.previous
#endif
#endif
