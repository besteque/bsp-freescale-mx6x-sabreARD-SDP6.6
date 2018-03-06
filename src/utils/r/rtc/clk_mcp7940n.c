/*
 * $QNXLicenseC:
 * Copyright 2015, QNX Software Systems.
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

#include "rtc.h"
#include <time.h>
#include <fcntl.h>
#include <hw/i2c.h>

/*
 * Microchip MCP7940N Serial RTC
 */
#define MCP7940N_SEC          0   /* 00-59 */
#define MCP7940N_MIN          1   /* 00-59 */
#define MCP7940N_HOUR         2   /* 0-1/00-23 */
#define MCP7940N_DAY          3   /* 01-07 */
#define MCP7940N_DATE         4   /* 01-31 */
#define MCP7940N_MONTH        5   /* 01-12 */
#define MCP7940N_YEAR         6   /* 00-99 */
#define MCP7940N_CONTROL      7
#define MCP7940N_OSCTRIM      8


#define MCP7940N_I2C_ADDRESS  (0xDE >> 1)
#define MCP7940N_I2C_DEVNAME  "/dev/i2c0"


static int fd = -1;

static int
mcp7940n_i2c_read(unsigned char reg, unsigned char val[], unsigned char num)
{
    iov_t           siov[2], riov[2];
    i2c_sendrecv_t  hdr;

    hdr.slave.addr = MCP7940N_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.send_len = 1;
    hdr.recv_len = num;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &reg, sizeof(reg));

    SETIOV(&riov[0], &hdr, sizeof(hdr));
    SETIOV(&riov[1], val, num);

    return devctlv(fd, DCMD_I2C_SENDRECV, 2, 2, siov, riov, NULL);
}

static int
mcp7940n_i2c_write(unsigned char reg, unsigned char val[], unsigned char num)
{
    iov_t           siov[3];
    i2c_send_t      hdr;

    hdr.slave.addr = MCP7940N_I2C_ADDRESS;
    hdr.slave.fmt = I2C_ADDRFMT_7BIT;
    hdr.len = num + 1;
    hdr.stop = 1;

    SETIOV(&siov[0], &hdr, sizeof(hdr));
    SETIOV(&siov[1], &reg, sizeof(reg));
    SETIOV(&siov[2], val, num);

    return devctlv(fd, DCMD_I2C_SEND, 3, 0, siov, NULL, NULL);
}

int
RTCFUNC(init,mcp7940n)(struct chip_loc *chip, char *argv[])
{
	char	*dev = (argv && argv[0] && argv[0][0]) ?  argv[0] : MCP7940N_I2C_DEVNAME;
    fd = open(dev, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Unable to open I2C device %s\n", dev);
        return -1;
    }

    return 0;
}

int
RTCFUNC(get,mcp7940n)(struct tm *tm, int cent_reg)
{
    unsigned char   date[7];
	int	st;

    st = mcp7940n_i2c_read(MCP7940N_SEC, date, 7);
	if(st < 0)
		st = mcp7940n_i2c_read(MCP7940N_SEC, date, 7); // allow 1 retry
	if(st < 0)
		return st;

    tm->tm_sec  = BCD2BIN(date[MCP7940N_SEC] & 0x7f);
    tm->tm_min  = BCD2BIN(date[MCP7940N_MIN] & 0x7f);

	if ((date[MCP7940N_HOUR] & 0x40)) {
		/* the rtc is in 12 hour mode */
		int hour = BCD2BIN(date[MCP7940N_HOUR] & 0x1f);

		if ((date[MCP7940N_HOUR] & 0x20))
			tm->tm_hour = (hour == 12) ? 12 : hour + 12; /* pm */
		else
			tm->tm_hour = (hour == 12) ? 0 : hour;  	 /* am */

	} else {
		/* rejoice! the rtc is in 24 hour mode */
     	tm->tm_hour = BCD2BIN(date[MCP7940N_HOUR] & 0x3f);
	}

    tm->tm_mday = BCD2BIN(date[MCP7940N_DATE] & 0x3f);
    tm->tm_mon  = BCD2BIN(date[MCP7940N_MONTH] & 0x1f) - 1;
    tm->tm_year = BCD2BIN(date[MCP7940N_YEAR] & 0xff);

    if(tm->tm_year < 70) tm->tm_year += 100;

    tm->tm_wday = BCD2BIN(date[MCP7940N_DAY] & 0x7) - 1;

    return(0);
}

int
RTCFUNC(set,mcp7940n)(struct tm *tm, int cent_reg)
{
    unsigned char   date[8];
	int	st;

    st = mcp7940n_i2c_read(MCP7940N_SEC, date, 8);
	if(st < 0)
		return st;
	if((date[MCP7940N_SEC]&0x80) == 0 && (date[MCP7940N_CONTROL]&0x08) == 0) {
		fprintf(stderr, "Warning: mcp7940n clock not enabled, configuring for crystal oscillator source with Vbat enabled\n");
		date[MCP7940N_SEC] |= 0x80; /* default to XTAL Oscillator if no clock enabled */
		date[MCP7940N_DAY] |= 0x08; /* default to Vbat Enable */
	}

	/*
	 * Note: this function will set the clock incorrectly after 2099,
	 * and it sets the clock in 24 hour mode.
	 */

    date[MCP7940N_SEC]   = BIN2BCD(tm->tm_sec) | (date[MCP7940N_SEC] & 0x80);  /* preserve Start Clock */
    date[MCP7940N_MIN]   = BIN2BCD(tm->tm_min);
    date[MCP7940N_HOUR]  = BIN2BCD(tm->tm_hour);
    date[MCP7940N_DATE]  = BIN2BCD(tm->tm_mday);
    date[MCP7940N_MONTH] = BIN2BCD(tm->tm_mon + 1);
    date[MCP7940N_YEAR]  = BIN2BCD(tm->tm_year % 100);
    date[MCP7940N_DAY]   = BIN2BCD(tm->tm_wday + 1) | (date[MCP7940N_DAY] & 0x08); /* preserve Vbat Enable */

    return mcp7940n_i2c_write(MCP7940N_SEC, date, 7); /* don't write CONTROL(0x7) reg */
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/utils/r/rtc/clk_mcp7940n.c $ $Rev: 800931 $")
#endif
