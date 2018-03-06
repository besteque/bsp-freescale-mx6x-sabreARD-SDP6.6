/*
 * $QNXLicenseC: 
 * Copyright 2011, QNX Software Systems.  
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

#include "ipl_mx6x.h"
#include <sys/srcversion.h>
#include <arm/mx6x_iomux.h>
#include <hw/inout.h>
#include <gulliver.h>


#define MX6Q_PAD_SETTINGS_UART (PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE | \
                				PAD_CTL_PUS_100K_PU | PAD_CTL_SPEED_MEDIUM | \
                                PAD_CTL_DSE_40_OHM | PAD_CTL_SRE_FAST | PAD_CTL_PUE_PULL)

#define MX6Q_PAD_SETTINGS_USDHC (PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_PUS_47K_PU | \
                                PAD_CTL_SPEED_LOW | PAD_CTL_DSE_80_OHM | PAD_CTL_SRE_FAST | \
                                PAD_CTL_HYS_ENABLE)

#define MX6Q_PAD_SETTINGS_ECSPI (PAD_CTL_SRE_FAST | PAD_CTL_SPEED_MEDIUM | PAD_CTL_DSE_40_OHM | PAD_CTL_HYS_ENABLE)

#define MX6Q_PAD_SETTINGS_I2C   (PAD_CTL_SRE_FAST | PAD_CTL_ODE_ENABLE | PAD_CTL_PKE_ENABLE | \
                            	PAD_CTL_PUE_PULL | PAD_CTL_DSE_40_OHM | PAD_CTL_PUS_100K_PU | \
							    PAD_CTL_HYS_ENABLE | PAD_CTL_SPEED_MEDIUM)

#define MX6Q_PAD_SETTINGS_GPI_SWITCH (PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_PUS_22K_PU | \
                                PAD_CTL_SPEED_LOW | PAD_CTL_SRE_SLOW | PAD_CTL_HYS_ENABLE)

extern int get_imx6_type();


static inline void set_mux_cfg(unsigned int pin, unsigned int value)
{
	out32((MX6X_IOMUXC_BASE + MX6X_IOMUX_SWMUX + (pin * 4)), value);
}

static inline void set_pad_cfg(unsigned int pin, unsigned int value)
{
	out32((MX6X_IOMUXC_BASE + MX6X_IOMUX_SWPAD + (pin * 4)), value);
}

static inline void set_pin_as_input(unsigned int pin, unsigned int value)
{
	unsigned offset;

	offset = 0;
	if ( get_imx6_type() == MX6_CHIP_TYPE_QUAD_OR_DUAL)
	{
		offset = MX6X_IOMUXC_BASE + MX6X_IOMUX_SWINPUT + (pin * 4);
	}
	else if (get_imx6_type() == MX6_CHIP_TYPE_DUAL_LITE_OR_SOLO)
	{
		offset = MX6X_IOMUXC_BASE + MX6SDL_IOMUX_SWINPUT + (pin * 4);
	}
	out32(offset, value);
}

void apply_mx6_sdl_dcd_values(unsigned long dcd_table_ptr)
{
	/*
	 * There is no mechanism in the iMX6 bootloader to read different
	 * DCD tables based on the CPU type, so we set up the DCD table with
	 * values for a quad/dual processor and overwrite afterwards
	 * with DualLite/Solo values
	 */
	_Uint32t num_table_entries;
	unsigned long i;
	_Uint32t target_address;
	_Uint32t value;

	/*
	 * Get number of table entries: first get overall size in bytes,
	 * then derive the number of entries.
	 */
	num_table_entries  = *(unsigned long *)dcd_table_ptr;
	ENDIAN_SWAP32(&num_table_entries);
	num_table_entries = (num_table_entries & 0x00FFFF00)>>8;
	num_table_entries = (num_table_entries - 8) / 8; /* (tbl_sz-header)/entry_size */

	/* skip the whole header - we assume all commands are writes */
	dcd_table_ptr += 8;
	target_address = *(unsigned long *)dcd_table_ptr;
	value = *(unsigned long *)(dcd_table_ptr+4);
	ENDIAN_SWAP32(&target_address);
	ENDIAN_SWAP32(&value);

	/* Apply value for each entry */
	for (i=0;i<num_table_entries;i++) \
	{
		target_address = *(unsigned long *)dcd_table_ptr;
		value = *(unsigned long *)(dcd_table_ptr+4);

		/* Endianess is wrong - swap endian */
		ENDIAN_SWAP32(&target_address);
		ENDIAN_SWAP32(&value);

		*(unsigned long *)target_address = value;
		dcd_table_ptr += 8;
	}
} /* End apply_mx6_sdl_dcd_values() */

void init_clocks(void)
{
    /* Restore the default values in the Gate registers */
	out32((MX6X_CCM_BASE + MX6X_CCM_CCGR0), MX6X_CCM_CCGR0_RESET);
	out32((MX6X_CCM_BASE + MX6X_CCM_CCGR1), MX6X_CCM_CCGR1_RESET);
	out32((MX6X_CCM_BASE + MX6X_CCM_CCGR2), MX6X_CCM_CCGR2_RESET);
	out32((MX6X_CCM_BASE + MX6X_CCM_CCGR3), MX6X_CCM_CCGR3_RESET);
	out32((MX6X_CCM_BASE + MX6X_CCM_CCGR4), MX6X_CCM_CCGR4_RESET);
	out32((MX6X_CCM_BASE + MX6X_CCM_CCGR5), MX6X_CCM_CCGR5_RESET);
	out32((MX6X_CCM_BASE + MX6X_CCM_CCGR6), MX6X_CCM_CCGR6_RESET);
	out32((MX6X_CCM_BASE + MX6X_CCM_CCGR7), MX6X_CCM_CCGR7_RESET);

    /* Gate/ungate PFDs to ensure proper reset (iMX6 D/Q Erratum ERR006282) */
    out32(MX6X_ANATOP_BASE + MX6X_ANATOP_PFD_480_SET, MX6X_ANATOP_PFD_ALL_CLOCK_GATES);
    
    /* Only reset 528PFD2 on the Q/D chip, as done by the 
       Freescale "ENGR00235821 mx6: correct work flow of PFDs" patch 
    */
    if ( get_imx6_type() == MX6_CHIP_TYPE_QUAD_OR_DUAL)
    {
        out32(MX6X_ANATOP_BASE + MX6X_ANATOP_PFD_528_SET, MX6X_ANATOP_PFD_ALL_CLOCK_GATES);
    }
    else
    {
        out32( MX6X_ANATOP_BASE + MX6X_ANATOP_PFD_528_SET, \
               (MX6X_ANATOP_PFD_ALL_CLOCK_GATES & ~(1 << MX6X_ANATOP_PFD2_GATE)) );
    }

    out32(MX6X_ANATOP_BASE + MX6X_ANATOP_PFD_480_CLR, MX6X_ANATOP_PFD_ALL_CLOCK_GATES);
    if ( get_imx6_type() == MX6_CHIP_TYPE_QUAD_OR_DUAL)
    {
        out32(MX6X_ANATOP_BASE + MX6X_ANATOP_PFD_528_CLR, MX6X_ANATOP_PFD_ALL_CLOCK_GATES);
    }
    else
    {
        out32( MX6X_ANATOP_BASE + MX6X_ANATOP_PFD_528_CLR, \
               (MX6X_ANATOP_PFD_ALL_CLOCK_GATES & ~(1 << MX6X_ANATOP_PFD2_GATE)) );
    }

 }

void init_regulator_settings(void)
{
	unsigned int drcr_value;

    /*
     * Set the Digital Regulator Core Register (DRCR) - it trims the core
     * voltage rails internal to the iMX6
     */
	drcr_value = in32(MX6X_ANATOP_BASE + MX6X_ANADIG_REG_CORE);

	/*
	 *  Adjust FET power gating for Regulator 2 (SOC domain)
	 *  to a core voltage of 1.2V
	 */
	drcr_value &= ~ANADIG_REG_CORE_REG2_TRG_MASK;
	drcr_value |= ((0x14 << 18) & ANADIG_REG_CORE_REG2_TRG_MASK);

	out32(MX6X_ANATOP_BASE + MX6X_ANADIG_REG_CORE, drcr_value);
}

void init_pinmux_quad_dual()
{

    /* UART4 TX - pin KEY_COL0 */
	set_mux_cfg(SWMUX_KEY_COL0, MUX_CTL_MUX_MODE_ALT4);
	set_pad_cfg(SWPAD_KEY_COL0, MX6Q_PAD_SETTINGS_UART);

    /* UART4 RX - pin KEY_ROW0 */
	set_mux_cfg(SWMUX_KEY_ROW0, MUX_CTL_MUX_MODE_ALT4);
	set_pad_cfg(SWPAD_KEY_ROW0, MX6Q_PAD_SETTINGS_UART);
	set_pin_as_input(SWINPUT_UART4_IPP_UART_RXD_MUX, 0x01);

	/* UART2*/
	set_mux_cfg(SWMUX_GPIO_7, MUX_CTL_MUX_MODE_ALT4);
	set_pad_cfg(SWPAD_GPIO_7, MX6Q_PAD_SETTINGS_UART);
	set_pin_as_input(SWINPUT_UART2_IPP_UART_RXD_MUX, 0x2);

	set_mux_cfg(SWMUX_GPIO_8, MUX_CTL_MUX_MODE_ALT4);
	set_pad_cfg(SWPAD_GPIO_8, MX6Q_PAD_SETTINGS_UART);
	set_pin_as_input(SWINPUT_UART2_IPP_UART_RXD_MUX, 0x3);

	set_mux_cfg(SWMUX_SD4_DAT6, MUX_CTL_MUX_MODE_ALT2);
	set_pad_cfg(SWPAD_SD4_DAT6, MX6Q_PAD_SETTINGS_UART);
	set_pin_as_input(SWINPUT_UART2_IPP_UART_RTS_B, 0x5);

	set_mux_cfg(SWMUX_SD4_DAT5, MUX_CTL_MUX_MODE_ALT2);
	set_pad_cfg(SWPAD_SD4_DAT5, MX6Q_PAD_SETTINGS_UART);
	set_pin_as_input(SWINPUT_UART2_IPP_UART_RTS_B, 0x4);

	/*
     * USDHC interfaces
     */

	/***********
	 * SD1
	 ***********/

	/* SD1 CLK */
	set_mux_cfg(SWMUX_SD1_CLK, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD1_CLK, MX6Q_PAD_SETTINGS_USDHC);

	/* SD1 CMD */
	set_mux_cfg(SWMUX_SD1_CMD, MUX_CTL_MUX_MODE_ALT0 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_SD1_CMD, MX6Q_PAD_SETTINGS_USDHC);

	/* SD1 DAT0 */
	set_mux_cfg(SWMUX_SD1_DAT0, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD1_DAT0, MX6Q_PAD_SETTINGS_USDHC);

	/* SD1 DAT1 */
	set_mux_cfg(SWMUX_SD1_DAT1, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD1_DAT1, MX6Q_PAD_SETTINGS_USDHC);

	/* SD1 DAT2 */
	set_mux_cfg(SWMUX_SD1_DAT2, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD1_DAT2, MX6Q_PAD_SETTINGS_USDHC);

	/* SD1 DAT3 */
	set_mux_cfg(SWMUX_SD1_DAT3, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD1_DAT3, MX6Q_PAD_SETTINGS_USDHC);

	/* SD1 Card Detect - configure GPIO1[1] as an input */
	set_mux_cfg(SWMUX_GPIO_1, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) & ~(1<<1));

	/* SD1 Write Protect - configure GPIO5[20] as an input */
	set_mux_cfg(SWMUX_CSI0_DATA_EN, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) & ~(1<<20));

	/***********
	 * SD3
	 ***********/

	/* SD3 CLK */
	set_mux_cfg(SWMUX_SD3_CLK, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD3_CLK, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 CMD */
	set_mux_cfg(SWMUX_SD3_CMD, MUX_CTL_MUX_MODE_ALT0 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_SD3_CMD, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 DAT0 */
	set_mux_cfg(SWMUX_SD3_DAT0, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD3_DAT0, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 DAT1 */
	set_mux_cfg(SWMUX_SD3_DAT1, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD3_DAT1, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 DAT2 */
	set_mux_cfg(SWMUX_SD3_DAT2, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD3_DAT2, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 DAT3 */
	set_mux_cfg(SWMUX_SD3_DAT3, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD3_DAT3, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 DAT4 */
	set_mux_cfg(SWMUX_SD3_DAT4, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD3_DAT4, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 DAT5 */
	set_mux_cfg(SWMUX_SD3_DAT5, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD3_DAT5, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 DAT6 */
	set_mux_cfg(SWMUX_SD3_DAT6, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD3_DAT6, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 DAT7 */
	set_mux_cfg(SWMUX_SD3_DAT7, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SD3_DAT7, MX6Q_PAD_SETTINGS_USDHC);

	/* SD3 Write Protect - configure GPIO1[13] as an input */
	set_mux_cfg(SWMUX_SD2_DAT2, MUX_CTL_MUX_MODE_ALT5);
//	set_pad_cfg(SWPAD_SD2_DAT2, MX6Q_PAD_SETTINGS_USDHC_WP);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) & ~(1<<13));

	/* SD3 Card Detect - configure GPIO6[15] as an input */
	set_mux_cfg(SWMUX_NANDF_CS2, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO6_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO6_BASE + MX6X_GPIO_GDIR) & ~(1<<15));

	/*
	* SPI
	*/

	/* SPI SCLK */
	set_mux_cfg(SWMUX_EIM_D16, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_EIM_D16, MX6Q_PAD_SETTINGS_ECSPI);
	set_pin_as_input(SWINPUT_ECSPI1_IPP_CSPI_CLK, 0x0);

	/* SPI MISO */
	set_mux_cfg(SWMUX_EIM_D17, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_EIM_D17, MX6Q_PAD_SETTINGS_ECSPI);
	set_pin_as_input(SWINPUT_ECSPI1_IPP_IND_MISO, 0x0);

	/* SPI MOSI */
	set_mux_cfg(SWMUX_EIM_D18, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_EIM_D18, MX6Q_PAD_SETTINGS_ECSPI);
	set_pin_as_input(SWINPUT_ECSPI1_IPP_IND_MOSI, 0x0);

	/* Select mux mode ALT1 for SS1 */
	set_mux_cfg(SWMUX_EIM_D19, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_EIM_D19, MX6Q_PAD_SETTINGS_ECSPI);
	set_pin_as_input(SWINPUT_ECSPI1_IPP_IND_SS_B_1, 0x0);

	out32(MX6X_GPIO5_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_DR) & ~(1<<4));  /* Enable I2C3 SDA route path */

	/*
	 * I2C
	 */

	/* I2C2  SCL */
	set_mux_cfg(SWMUX_EIM_EB2, MUX_CTL_MUX_MODE_ALT6 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_EIM_EB2, MX6Q_PAD_SETTINGS_I2C);
	set_pin_as_input(SWINPUT_I2C2_IPP_SCL_IN, 0x0);

	/* I2C2  SDA */
	set_mux_cfg(SWMUX_KEY_ROW3, MUX_CTL_MUX_MODE_ALT4 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_KEY_ROW3, MX6Q_PAD_SETTINGS_I2C);
	set_pin_as_input(SWINPUT_I2C2_IPP_SDA_IN, 0x1);

	/* I2C3 SCL */
	set_mux_cfg(SWMUX_GPIO_3, MUX_CTL_MUX_MODE_ALT2 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_GPIO_3, MX6Q_PAD_SETTINGS_I2C);
	set_pin_as_input(SWINPUT_I2C3_IPP_SCL_IN, 1);

	/* I2C3  SDA */
	set_mux_cfg(SWMUX_EIM_D18, MUX_CTL_MUX_MODE_ALT6 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_EIM_D18, MX6Q_PAD_SETTINGS_I2C);
	set_pin_as_input(SWINPUT_I2C3_IPP_SDA_IN, 0);

	/* EIM_A24__GPIO_5_4 steer logic enable */
	set_mux_cfg(SWMUX_EIM_A24, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) | (1<<4)); /* set as an output */
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_DR) | (1<<4));  /* Enable I2C3 SDA route path */

	/* GPIO1[15] */
	set_mux_cfg(SWMUX_SD2_DAT0, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) | (1<<15)); //set as an output
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) & ~(1<<15)); // low
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) | (1<<15)); // high

} /* End init_pinmux_quad_dual() */


void init_pinmux_dual_lite_solo()
{

    /* UART1 TX - pin CSI0_DAT10 */
	set_mux_cfg(SWMUX_SDL_CSI0_DAT10, MUX_CTL_MUX_MODE_ALT3);
	set_pad_cfg(SWPAD_SDL_CSI0_DAT10, MX6Q_PAD_SETTINGS_UART);
	set_pin_as_input(SWINPUT_SDL_UART1_IPP_UART_RXD_MUX, 0);

	/* UART1 RX - pin CSI0_DAT11 */
	set_mux_cfg(SWMUX_SDL_CSI0_DAT11, MUX_CTL_MUX_MODE_ALT3);
	set_pad_cfg(SWPAD_SDL_CSI0_DAT11, MX6Q_PAD_SETTINGS_UART);
	set_pin_as_input(SWINPUT_SDL_UART1_IPP_UART_RXD_MUX, 1);

	/*
     * USDHC interfaces
     */

	/***********
	 * SD4
	 ***********/

	/* SD4 CLK */
	set_mux_cfg(SWMUX_SDL_SD4_CLK, MUX_CTL_MUX_MODE_ALT0);
	set_pad_cfg(SWPAD_SDL_SD4_CLK, MX6Q_PAD_SETTINGS_USDHC);

	/* SD4 CMD */
	set_mux_cfg(SWMUX_SDL_SD4_CMD, MUX_CTL_MUX_MODE_ALT0 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_SDL_SD4_CMD, MX6Q_PAD_SETTINGS_USDHC);

	/* SD4 DAT0 */
	set_mux_cfg(SWMUX_SDL_SD4_DAT0, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_SD4_DAT0, MX6Q_PAD_SETTINGS_USDHC);

	/* SD4 DAT1 */
	set_mux_cfg(SWMUX_SDL_SD4_DAT1, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_SD4_DAT1, MX6Q_PAD_SETTINGS_USDHC);

	/* SD4 DAT2 */
	set_mux_cfg(SWMUX_SDL_SD4_DAT2, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_SD4_DAT2, MX6Q_PAD_SETTINGS_USDHC);

	/* SD4 DAT3 */
	set_mux_cfg(SWMUX_SDL_SD4_DAT3, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_SD4_DAT3, MX6Q_PAD_SETTINGS_USDHC);

	/* SD4 DAT4 */
	set_mux_cfg(SWMUX_SDL_SD4_DAT4, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_SD4_DAT4, MX6Q_PAD_SETTINGS_USDHC);

	/* SD4 DAT5 */
	set_mux_cfg(SWMUX_SDL_SD4_DAT5, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_SD4_DAT5, MX6Q_PAD_SETTINGS_USDHC);

	/* SD4 DAT6 */
	set_mux_cfg(SWMUX_SDL_SD4_DAT6, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_SD4_DAT6, MX6Q_PAD_SETTINGS_USDHC);

	/* SD4 DAT7 */
	set_mux_cfg(SWMUX_SDL_SD4_DAT7, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_SD4_DAT7, MX6Q_PAD_SETTINGS_USDHC);

#if 0  //for sd card
	/*
	 * SD4 Write Protect - configure GPIO1[13] as an input
	 * Note: changed default pad settings to settings appropriate to a switch
	 */
	set_mux_cfg(SWMUX_SDL_SD2_DAT2, MUX_CTL_MUX_MODE_ALT5);
	set_pad_cfg(SWPAD_SDL_SD2_DAT2, MX6Q_PAD_SETTINGS_GPI_SWITCH);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) & ~(1<<13));

	/*
	 * SD4 Card Detect - configure GPIO6[15] as an input
	 * Note: changed default pad settings to settings appropriate to a switch
	 */
	set_mux_cfg(SWMUX_SDL_NANDF_CS2, MUX_CTL_MUX_MODE_ALT5);
	set_pad_cfg(SWPAD_SDL_NANDF_CS2, MX6Q_PAD_SETTINGS_GPI_SWITCH);
	out32(MX6X_GPIO6_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO6_BASE + MX6X_GPIO_GDIR) & ~(1<<15));
#endif

	/*
	* SPI
	*/

	/* SPI SCLK */
	set_mux_cfg(SWMUX_SDL_EIM_D16, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_EIM_D16, MX6Q_PAD_SETTINGS_ECSPI);
	set_pin_as_input(SWINPUT_SDL_ECSPI1_IPP_CSPI_CLK, 0x0);

	/* SPI MISO - input daisy chain changed with Solo */
	set_mux_cfg(SWMUX_SDL_EIM_D17, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_EIM_D17, MX6Q_PAD_SETTINGS_ECSPI);
	set_pin_as_input(SWINPUT_SDL_ECSPI1_IPP_IND_MISO, 0x2);

	/* SPI MOSI - input daisy chain changed with Solo */
	set_mux_cfg(SWMUX_SDL_EIM_D18, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_EIM_D18, MX6Q_PAD_SETTINGS_ECSPI);
	set_pin_as_input(SWINPUT_SDL_ECSPI1_IPP_IND_MOSI, 0x2);

	/* Select mux mode ALT1 for SS1 - input daisy chain changed with Solo */
	set_mux_cfg(SWMUX_SDL_EIM_D19, MUX_CTL_MUX_MODE_ALT1);
	set_pad_cfg(SWPAD_SDL_EIM_D19, MX6Q_PAD_SETTINGS_ECSPI);
	set_pin_as_input(SWINPUT_SDL_ECSPI1_IPP_IND_SS_B_1, 0x1);

	/*
	 * I2C
	 */

	/* I2C3 steering logic - preset EIM_A24__GPIO_5_4 Data Reg to 0 (disable) */
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_DR) & ~(1<<4));

	/* I2C2  SCL */
	set_mux_cfg(SWMUX_SDL_EIM_EB2, MUX_CTL_MUX_MODE_ALT6 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_SDL_EIM_EB2, MX6Q_PAD_SETTINGS_I2C);
	set_pin_as_input(SWINPUT_SDL_I2C2_IPP_SCL_IN, 0x0);

	/* I2C2  SDA */
	set_mux_cfg(SWMUX_SDL_KEY_ROW3, MUX_CTL_MUX_MODE_ALT4 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_SDL_KEY_ROW3, MX6Q_PAD_SETTINGS_I2C);
	set_pin_as_input(SWINPUT_SDL_I2C2_IPP_SDA_IN, 0x1);

	/* I2C3 SCL */
	set_mux_cfg(SWMUX_SDL_GPIO_3, MUX_CTL_MUX_MODE_ALT2 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_SDL_GPIO_3, MX6Q_PAD_SETTINGS_I2C);
	set_pin_as_input(SWINPUT_SDL_I2C3_IPP_SCL_IN, 1);

	/* I2C3  SDA */
	set_mux_cfg(SWMUX_SDL_EIM_D18, MUX_CTL_MUX_MODE_ALT6 | MUX_CTL_SION);
	set_pad_cfg(SWPAD_SDL_EIM_D18, MX6Q_PAD_SETTINGS_I2C);
	set_pin_as_input(SWINPUT_SDL_I2C3_IPP_SDA_IN, 0);

	/* EIM_A24__GPIO_5_4 GPIO5[4]: I2C3 steering logic enable */
	set_mux_cfg(SWMUX_SDL_EIM_A24, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) | (1<<4)); /* set as an output */
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_DR) | (1<<4));  /* Enable I2C3 SDA route path */

	/*
	 * GPIO1[15]: I2C port expander reset (active low)
	 *            I2C port expander controls the USB OTG Power On, Sat reset and NAND BT Wifi steer
	 *            Toggle the port expander reset signal
	 */
	set_mux_cfg(SWMUX_SDL_SD2_DAT0, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) | (1<<15)); //set as an output
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) & ~(1<<15)); // low
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) | (1<<15)); // high
} /* End init_pinmux_dual_lite_solo() */

void init_aips()
{
   /*
     * Set all MPROTx to be non-bufferable, trusted for R/W,
     * not forced to user-mode.
     */

	/* AIPS1 registers */
	out32((MX6X_AIPS1_CONFIG + 0x00), 0x77777777);

	/* Register only exists for Quad and Dual processors */
	if ( get_imx6_type() == MX6_CHIP_TYPE_QUAD_OR_DUAL)
	{
		out32((MX6X_AIPS1_CONFIG + 0x04), 0x77777777);
	}
	out32((MX6X_AIPS1_CONFIG + 0x40), 0x0);
	out32((MX6X_AIPS1_CONFIG + 0x44), 0x0);
	out32((MX6X_AIPS1_CONFIG + 0x48), 0x0);
	out32((MX6X_AIPS1_CONFIG + 0x4C), 0x0);
	out32((MX6X_AIPS1_CONFIG + 0x50), 0x0);


	/* AIPS2 registers */
	out32((MX6X_AIPS2_CONFIG + 0x00), 0x77777777);

	/* Register only exists for Quad and Dual processors */
	if ( get_imx6_type() == MX6_CHIP_TYPE_QUAD_OR_DUAL)
	{
		out32((MX6X_AIPS2_CONFIG + 0x04), 0x77777777);
	}
	out32((MX6X_AIPS2_CONFIG + 0x40), 0x0);
	out32((MX6X_AIPS2_CONFIG + 0x44), 0x0);
	out32((MX6X_AIPS2_CONFIG + 0x48), 0x0);
	out32((MX6X_AIPS2_CONFIG + 0x4C), 0x0);
	out32((MX6X_AIPS2_CONFIG + 0x50), 0x0);
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/ipl/boards/mx6x-sabreARD/init.c $ $Rev: 718378 $")
#endif
