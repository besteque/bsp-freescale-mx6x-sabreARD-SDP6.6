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

#include "startup.h"
#include <arm/sc58x.h>

static void parse_line(unsigned channel, const char *line, unsigned *baud, unsigned *clk) {
    if (*line != '.' && *line != '\0') {
        dbg_device[channel].base = strtopaddr(line, (char **)&line, 16);
        if (*line == '^') dbg_device[channel].shift = strtoul(line+1, (char **)&line, 0);
    }
    if (*line == '.') ++line;
    if (*line != '.' && *line != '\0') *baud = strtoul(line, (char **)&line, 0);
    if (*line == '.') ++line;
    if (*line != '.' && *line != '\0') *clk = strtoul(line, (char **)&line, 0);
}

void init_sersc58x(unsigned channel, const char *init, const char *defaults)
{
    unsigned    baud = 0;
    unsigned    clk;
    unsigned    base;

    dbg_device[channel].base = SC58X_UART0_BASE;    // default UART0
    parse_line(channel, defaults, &baud, &clk);
    parse_line(channel, init, &baud, &clk);

    base = dbg_device[channel].base;

	// Wait for all pending characters to be output...
	do {
	} while(!(in32(base + SC58X_UART_STAT) & UART_STAT_TEMT));

    if (baud != 0)
        out32(base + SC58X_UART_CLK, clk / baud / 16);

    // 8 bit, no parity
    out32(base + SC58X_UART_CTRL, UART_CTRL_WLS8 | UART_CTRL_EN);
}

void put_sc58x(int c)
{
    unsigned base = dbg_device[0].base;

    while (!(in32(base + SC58X_UART_STAT) & UART_STAT_THRE))
        ;
    out32(base + SC58X_UART_THR, c);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/lib/arm/hw_sersc58x.c $ $Rev: 801973 $")
#endif

