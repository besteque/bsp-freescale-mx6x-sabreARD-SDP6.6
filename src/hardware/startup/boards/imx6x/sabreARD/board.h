/*
 * $QNXLicenseC: 
 * Copyright 2012, QNX Software Systems.  
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


#ifndef __BOARD_H
#define __BOARD_H

#include <arm/mx6x_iomux.h>
#include <arm/mx6x.h>
#include "mx6x_startup.h"

/* Function declarations centralized here */
extern struct callout_rtn reboot_mx6x;
extern void mx6q_usb_otg_host_init(void);
extern void mx6sdl_usb_otg_host_init(void);
extern void mx6q_init_ecspi(void);
extern void mx6sdl_init_ecspi(void);
extern void mx6sdl_init_i2c(void);
extern void mx6q_init_i2c2(void);
extern void mx6sdl_init_i2c2(void);
extern void mx6q_init_i2c3(void);
extern void mx6sdl_init_i2c3(void);
extern void mx6q_init_usdhc(void);
extern void mx6sdl_init_usdhc(void);
extern void mx6q_init_lvds0(void);
extern void mx6sdl_init_lvds0(void);
extern void mx6q_init_lvds1(void);
extern void mx6sdl_init_lvds1(void);
extern void mx6q_init_lcd_panel(void);
extern void mx6sdl_init_lcd_panel(void);
extern void mx6q_init_can(void);
extern void mx6sdl_init_can(void);
extern void mx6q_init_enet(void);
extern void mx6sdl_init_enet(void);
extern void mx6q_init_uart_2(void);
extern void mx6sdl_init_uart_2(void);
extern void mx6q_init_uart_3(void);
extern void mx6sdl_init_uart_3(void);
extern void mx6q_init_esai(void);
extern void mx6sdl_init_esai(void);
extern void mx6q_init_spdif(void);
extern void mx6sdl_init_spdif(void);
extern void mx6q_init_pmic(void);
extern void mx6sdl_init_pmic(void);
extern void mx6x_init_mac(void);
extern void mx6x_nor_init(void);
extern void mx6x_init_displays(void);
extern void mx6x_usb_host1_init(void);
extern void init_enet_can_steer(void);
extern void init_nor_steer(void);
extern void init_video_steer(void);
extern void init_cpucard_reset_steer(void);
extern void init_mainboard_reset_steer(void);
extern void hwi_imx6dq();
extern void mx6q_init_video(void);
extern unsigned mx6x_init_lvds_clock(void);

extern void mx6sdl_init_uart_1(void);

/* I2C I/O expanders slave addresses */
#define IO_SLAVE_ADDR_A     0x30
#define IO_SLAVE_ADDR_B     0x32
#define IO_SLAVE_ADDR_C     0x34

/* OUTPUT PORT REG */
#define OUT_PORT_REG    0x1
#define OUT_PORT_0         (1<<0)
#define OUT_PORT_1         (1<<1)
#define OUT_PORT_2         (1<<2)
#define OUT_PORT_3         (1<<3)
#define OUT_PORT_4         (1<<4)
#define OUT_PORT_5         (1<<5)
#define OUT_PORT_6         (1<<6)
#define OUT_PORT_7         (1<<7)

/* CONFIGURATION REG */
#define CONF_REG       0x3
#define CONF_REG_0        (1<<0)
#define CONF_REG_1        (1<<1)
#define CONF_REG_2        (1<<2)
#define CONF_REG_3        (1<<3)
#define CONF_REG_4        (1<<4)
#define CONF_REG_5        (1<<5)
#define CONF_REG_6        (1<<6)
#define CONF_REG_7        (1<<7)


/* Clock: 66M, Divider: 768, I2C clock: 66M / 768 = 85937 */
#define I2C_CLK_DIV         0x15

/*
 * External DDR RAM size (MB) - note that the Sabre ARD board always has 2GB
 * of DDR3 RAM, but only 1 GB is acccessible with the Solo because of the
 * 32-bit bus limitation (vs 64-bit)
 */
#define MX6Q_D_DL_SDRAM_SIZE	2048
#define MX6S_SDRAM_SIZE			1024

/* disabled clocks in CCGR4 */
#define MX6X_DISABLE_CLOCK_CCGR4   CCGR4_CG0_PCIE

/* disabled clocks in CCGR5 */
#define MX6X_DISABLE_CLOCK_CCGR5 ( CCGR5_CG11_SSI3 | \
								   CCGR5_CG10_SSI2)

/* disabled clocks in CCGR7 */
#define MX6X_DISABLE_CLOCK_CCGR7  CCGR6_CG2_USDHC2


#endif

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/sabreARD/board.h $ $Rev: 751697 $")
#endif
