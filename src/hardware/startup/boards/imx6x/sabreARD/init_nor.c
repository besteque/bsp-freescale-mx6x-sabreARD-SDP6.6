/*
 * $QNXLicenseC: 
 * Copyright 2011, 2012 QNX Software Systems.  
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
#include "board.h"

#define MX6Q_PAD_SETTINGS_GENERAL (PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE | \
                				PAD_CTL_PUS_100K_PU | PAD_CTL_SPEED_MEDIUM | \
                                PAD_CTL_DSE_40_OHM | PAD_CTL_SRE_SLOW | PAD_CTL_PUE_PULL)

#define MX6Q_PAD_SETTINGS_WEIM1     MX6Q_PAD_SETTINGS_GENERAL

#define MX6Q_PAD_SETTINGS_WEIM2     (PAD_CTL_HYS_DISABLE | PAD_CTL_PUS_100K_PU | \
                                PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE | PAD_CTL_ODE_DISABLE | \
                                PAD_CTL_SPEED_MEDIUM | PAD_CTL_DSE_40_OHM | PAD_CTL_SRE_FAST)

#define MX6Q_PAD_SETTINGS_WEIM3     (PAD_CTL_HYS_DISABLE | PAD_CTL_PUS_100K_PU | \
                                PAD_CTL_PUE_PULL | PAD_CTL_PKE_ENABLE | PAD_CTL_ODE_DISABLE | \
                                PAD_CTL_SPEED_LOW| PAD_CTL_DSE_60_OHM | PAD_CTL_SRE_SLOW)

void mx6q_init_nor_mux(void)
{
	/*
	 * WEIM for the Parallel NOR Flash
	 */

	/* WEIM_DA_OE */
	pinmux_set_swmux(SWMUX_EIM_OE, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_OE, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_RW */
	pinmux_set_swmux(SWMUX_EIM_RW, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_RW, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_WAIT */
	pinmux_set_swmux(SWMUX_EIM_WAIT, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_WAIT, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_CS0 */
	pinmux_set_swmux(SWMUX_EIM_CS0, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_CS0, MX6Q_PAD_SETTINGS_WEIM2);

	/*Control NOR reset using gpio mode - GP4[29]*/
	pinmux_set_swmux(SWMUX_DISP0_DAT8, MUX_CTL_MUX_MODE_ALT5);

	out32(MX6X_GPIO4_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO4_BASE + MX6X_GPIO_GDIR) | (1<<29)); //set as an output
	out32(MX6X_GPIO4_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO4_BASE + MX6X_GPIO_DR) & ~(1<<29)); // low
	mx6x_usleep(1000);
	out32(MX6X_GPIO4_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO4_BASE + MX6X_GPIO_DR) | (1<<29)); // high

	/* EIM_LBA */
	pinmux_set_swmux(SWMUX_EIM_LBA, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_LBA, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_BCLK */
	pinmux_set_swmux(SWMUX_EIM_BCLK, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_BCLK, MX6Q_PAD_SETTINGS_WEIM2);

	/* Parallel Nor Data Bus */
	/* WEIM_D[16] */
	pinmux_set_swmux(SWMUX_EIM_D16, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D16, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[17] */
	pinmux_set_swmux(SWMUX_EIM_D17, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D17, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[18] */
	pinmux_set_swmux(SWMUX_EIM_D18, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D18, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[19] */
	pinmux_set_swmux(SWMUX_EIM_D19, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D19, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[20] */
	pinmux_set_swmux(SWMUX_EIM_D20, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D20, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[21] */
	pinmux_set_swmux(SWMUX_EIM_D21, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D21, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[22] */
	pinmux_set_swmux(SWMUX_EIM_D22, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D22, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[23] */
	pinmux_set_swmux(SWMUX_EIM_D23, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D23, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[24] */
	pinmux_set_swmux(SWMUX_EIM_D24, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D24, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[25] */
	pinmux_set_swmux(SWMUX_EIM_D25, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D25, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[26] */
	pinmux_set_swmux(SWMUX_EIM_D26, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D26, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[27] */
	pinmux_set_swmux(SWMUX_EIM_D27, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D27, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[28] */
	pinmux_set_swmux(SWMUX_EIM_D28, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D28, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[29] */
	pinmux_set_swmux(SWMUX_EIM_D29, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D29, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[30] */
	pinmux_set_swmux(SWMUX_EIM_D30, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D30, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[31] */
	pinmux_set_swmux(SWMUX_EIM_D31, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_D31, MX6Q_PAD_SETTINGS_WEIM1);

	/* Parallel Nor 25 bit Address Bus */
	/* WEIM_A[16] */
	pinmux_set_swmux(SWMUX_EIM_A16, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_A16, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[17] */
	pinmux_set_swmux(SWMUX_EIM_A17, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_A17, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[18] */
	pinmux_set_swmux(SWMUX_EIM_A18, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_A18, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[19] */
	pinmux_set_swmux(SWMUX_EIM_A19, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_A19, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[20] */
	pinmux_set_swmux(SWMUX_EIM_A20, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_A20, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[21] */
	pinmux_set_swmux(SWMUX_EIM_A21, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_A21, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[22] */
	pinmux_set_swmux(SWMUX_EIM_A22, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_A22, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[23] */
	pinmux_set_swmux(SWMUX_EIM_A23, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_A23, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[24] - GP5[4] */
	pinmux_set_swmux(SWMUX_EIM_A24, MUX_CTL_MUX_MODE_ALT5);
		out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) | (1<<4)); /* set as an output */

	/* WEIM_DA_A[9] */
	pinmux_set_swmux(SWMUX_EIM_DA9, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_DA9, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[10] */
	pinmux_set_swmux(SWMUX_EIM_DA10, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_DA10, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[11] */
	pinmux_set_swmux(SWMUX_EIM_DA11, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_DA11, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[12] */
	pinmux_set_swmux(SWMUX_EIM_DA12, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_DA12, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[13] */
	pinmux_set_swmux(SWMUX_EIM_DA13, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_DA13, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[14] */
	pinmux_set_swmux(SWMUX_EIM_DA14, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_DA14, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[15] */
	pinmux_set_swmux(SWMUX_EIM_DA15, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_EIM_DA15, MX6Q_PAD_SETTINGS_WEIM2);
}

void mx6sdl_init_nor_mux(void)
{
	/*
	 * WEIM for the Parallel NOR Flash
	 */

	/* WEIM_DA_OE */
	pinmux_set_swmux(SWMUX_SDL_EIM_OE, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_OE, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_RW */
	pinmux_set_swmux(SWMUX_SDL_EIM_RW, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_RW, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_WAIT */
	pinmux_set_swmux(SWMUX_SDL_EIM_WAIT, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_WAIT, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_CS0 */
	pinmux_set_swmux(SWMUX_SDL_EIM_CS0, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_CS0, MX6Q_PAD_SETTINGS_WEIM2);

	/*Control NOR reset using gpio mode - GP4[29]*/
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT8, MUX_CTL_MUX_MODE_ALT5);

	/* Pulse NOR reset line */
	out32(MX6X_GPIO4_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO4_BASE + MX6X_GPIO_GDIR) | (1<<29)); //set as an output
	out32(MX6X_GPIO4_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO4_BASE + MX6X_GPIO_DR) & ~(1<<29)); // low
	mx6x_usleep(1000);
	out32(MX6X_GPIO4_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO4_BASE + MX6X_GPIO_DR) | (1<<29)); // high

	/* EIM_LBA */
	pinmux_set_swmux(SWMUX_SDL_EIM_LBA, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_LBA, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_BCLK */
	pinmux_set_swmux(SWMUX_SDL_EIM_BCLK, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_BCLK, MX6Q_PAD_SETTINGS_WEIM2);

	/* Parallel Nor Data Bus */
	/* WEIM_D[16] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D16, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D16, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[17] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D17, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D17, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[18] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D18, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D18, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[19] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D19, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D19, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[20] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D20, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D20, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[21] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D21, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D21, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[22] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D22, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D22, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[23] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D23, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D23, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[24] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D24, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D24, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[25] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D25, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D25, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[26] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D26, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D26, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[27] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D27, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D27, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[28] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D28, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D28, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[29] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D29, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D29, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[30] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D30, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D30, MX6Q_PAD_SETTINGS_WEIM1);

	/* WEIM_D[31] */
	pinmux_set_swmux(SWMUX_SDL_EIM_D31, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D31, MX6Q_PAD_SETTINGS_WEIM1);

	/* Parallel Nor 25 bit Address Bus */
	/* WEIM_A[16] */
	pinmux_set_swmux(SWMUX_SDL_EIM_A16, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_A16, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[17] */
	pinmux_set_swmux(SWMUX_SDL_EIM_A17, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_A17, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[18] */
	pinmux_set_swmux(SWMUX_SDL_EIM_A18, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_A18, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[19] */
	pinmux_set_swmux(SWMUX_SDL_EIM_A19, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_A19, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[20] */
	pinmux_set_swmux(SWMUX_SDL_EIM_A20, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_A20, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[21] */
	pinmux_set_swmux(SWMUX_SDL_EIM_A21, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_A21, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[22] */
	pinmux_set_swmux(SWMUX_SDL_EIM_A22, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_A22, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[23] */
	pinmux_set_swmux(SWMUX_SDL_EIM_A23, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_A23, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_A[24] - GP5[4] */
	pinmux_set_swmux(SWMUX_SDL_EIM_A24, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) | (1<<4)); /* set as an output */

	/* WEIM_DA_A[9] */
	pinmux_set_swmux(SWMUX_SDL_EIM_DA9, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_DA9, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[10] */
	pinmux_set_swmux(SWMUX_SDL_EIM_DA10, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_DA10, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[11] */
	pinmux_set_swmux(SWMUX_SDL_EIM_DA11, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_DA11, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[12] */
	pinmux_set_swmux(SWMUX_SDL_EIM_DA12, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_DA12, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[13] */
	pinmux_set_swmux(SWMUX_SDL_EIM_DA13, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_DA13, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[14] */
	pinmux_set_swmux(SWMUX_SDL_EIM_DA14, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_DA14, MX6Q_PAD_SETTINGS_WEIM2);

	/* WEIM_DA_A[15] */
	pinmux_set_swmux(SWMUX_SDL_EIM_DA15, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_EIM_DA15, MX6Q_PAD_SETTINGS_WEIM2);
}

static void
mx6x_setup_weim_cs(void)
{
	unsigned int reg;

	reg = in32(MX6X_CCM_BASE + MX6X_CCM_CCGR6);
	reg |= 0x00000C00;
	out32(MX6X_CCM_BASE + MX6X_CCM_CCGR6, reg);

	out32(MX6X_EIM_BASE, 0x00620081);
	out32(MX6X_EIM_BASE + MX6X_EIM_CS0RCR1,0x1C022000);
	out32(MX6X_EIM_BASE + MX6X_EIM_CS0WCR1,0x0804a240);
}

void mx6x_nor_init(void)
{
	init_nor_steer();
	if ( get_mx6_chip_type() == MX6_CHIP_TYPE_QUAD_OR_DUAL)
	{
		mx6q_init_nor_mux();
	}
	else if ( get_mx6_chip_type() == MX6_CHIP_TYPE_DUAL_LITE_OR_SOLO)
	{
		mx6sdl_init_nor_mux();
	}
	mx6x_setup_weim_cs();
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/sabreARD/init_nor.c $ $Rev: 691538 $")
#endif
