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
 * ADI ADSP-SC58X specific timer support.
 * The Generic Timer0 is used.
 */

#include <startup.h>
#include <arm/sc58x.h>

#define SC58X_GPT_CLOCK     112500000
#define SC58X_GPT0_ID       0

static uintptr_t    sc58x_gpt_base;

#if 0
extern struct callout_rtn   timer_load_sc58x;
extern struct callout_rtn   timer_value_sc58x;
extern struct callout_rtn   timer_reload_sc58x;
#endif


static const struct callout_slot    timer_callouts[] = {
    { CALLOUT_SLOT(timer_load, _sc58x) },
    { CALLOUT_SLOT(timer_value, _sc58x) },
    { CALLOUT_SLOT(timer_reload, _sc58x) },
};

static unsigned
timer_start_sc58x()
{
    return in32(sc58x_gpt_base + SC58X_TIMER0_TMR_CNT(SC58X_GPT0_ID));
}

static unsigned
timer_diff_sc58x(unsigned start)
{
    unsigned now = in32(sc58x_gpt_base + SC58X_TIMER0_TMR_CNT(SC58X_GPT0_ID));

    return (unsigned)((int)now - (int)start);
}

/*
 * Use Timer 0
 */
void
init_qtime_sc58x(unsigned base, unsigned clock, int irq)
{
    struct qtime_entry  *qtime = alloc_qtime();

    sc58x_gpt_base = startup_io_map(0x100, base);

    if (clock == 0)
        clock = SC58X_GPT_CLOCK;
    if (irq == -1)
        irq = SC58X_IRQ_TIMER0_TMR0;

    // stop timer
    out16(sc58x_gpt_base + SC58X_TIMER0_STOP_SET, 1 << SC58X_GPT0_ID);
    out16(sc58x_gpt_base + SC58X_TIMER0_RUN_CLR,  1 << SC58X_GPT0_ID);

    out16(sc58x_gpt_base + SC58X_TIMER0_TMR_CFG(SC58X_GPT0_ID),
                    TMR_CFG_OUTDIS | TMR_CFG_MODE_PWM_C | TMR_CFG_PULSEHI | TMR_CFG_IRQ_PE);

    out32(base + SC58X_TIMER0_TMR_PER(SC58X_GPT0_ID), 0xFFFFFFFF);
    out32(base + SC58X_TIMER0_TMR_WID(SC58X_GPT0_ID), 0xFFFFFFFE);

    // start timer
    out16(sc58x_gpt_base + SC58X_TIMER0_RUN_SET,  1 << SC58X_GPT0_ID);

    timer_start = timer_start_sc58x;
    timer_diff  = timer_diff_sc58x;
    qtime->intr = irq;

    qtime->timer_rate  = (unsigned long)((uint64_t)1000000000 * (uint64_t)1000000 / (uint64_t)clock);
    qtime->timer_scale = -15;
    qtime->cycles_per_sec = (uint64_t)clock;

    add_callout_array(timer_callouts, sizeof(timer_callouts));
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/lib/arm/init_qtime_sc58x.c $ $Rev: 801973 $")
#endif
