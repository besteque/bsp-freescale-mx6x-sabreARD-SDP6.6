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

#include "ipl.h"
#include "sdmmc.h"
#include "fat-fs.h"
#include "sdhc_mx6x.h"
#include "ipl_mx6x.h"
#include <hw/inout.h>
#include <stdint.h>

// Add-on for serial port init
#include "ipl_mx6x.h"
#include <sys/srcversion.h>
#include <arm/mx6x_iomux.h>

/* Load IFS in external RAM */
#define QNX_LOAD_ADDR   		0x11800000

/* Macros for detection of CPU type */
#define MX6X_ANATOP_BASE 		0x020C8000
#define MX6X_ANADIG_CHIP_INFO 	0x260

extern  fs_info_t       fs_info;
extern  unsigned long	dcd_hdr_solo;
extern  unsigned long	dcd_hdr_quad;
extern  unsigned long	dcd_hdr_quadplus;

extern	void apply_mx6_sdl_dcd_values(unsigned long dcd_ptr);
extern	void init_aips();
extern	void init_clocks();
extern	void init_regulator_settings();
extern	void init_pinmux_quad_dual();
extern	void init_pinmux_dual_lite_solo();
extern	void init_serial_mx6x();

/*
 * Function to retrieve CPU type - used to create conditional paths for
 * delta settings
 */
unsigned int get_imx6_type()
{
	int chip_id = in32(MX6X_ANATOP_BASE + MX6X_ANADIG_CHIP_INFO);
	chip_id >>= 16;

	return chip_id;
}

uint32_t get_imx6_rev()
{
	uint32_t chip_rev = 0;
	uint32_t dig_prog = in32(MX6X_ANATOP_BASE + MX6X_ANADIG_CHIP_INFO);

	/* Encode major and minor chip rev fields into one byte. 
	   Upper nibble is major, lower is minor */
	uint32_t major = ((dig_prog & 0xFF00) >> 8) + 1;
	uint32_t minor = dig_prog & 0xFF;
	chip_rev = (major<< 4) | (minor);

	return chip_rev;
}

void delay(unsigned dly)
{
	volatile int j;

	while (dly--) {
		for (j = 0; j < 32; j++)
			;
	}
}

static inline int
sdmmc_load_file (unsigned address, const char *fn)
{
	mx6x_sdmmc_t	sdmmc;
	int				status;

	/*
	 * Initialize the SDMMC interface
	 */
	sdmmc.sdmmc_pbase = MX6X_USDHC4_BASE;      // SDMMC base address

	/* initialize the sdmmc interface and card */
	if (SDMMC_OK != sdmmc_init_ctrl(&sdmmc)) {
		return SDMMC_ERROR;
	}

	if (sdmmc_init_card(&sdmmc)) {
		return SDMMC_ERROR;
	}

	ser_putstr("Load QNX image from SDMMC...\n");
	//if (fat_read_mbr(&sdmmc, 0) != 0) {
	if (fat_read_mbr(&sdmmc, 1) != 0) {  //open dbg info
		return SDMMC_ERROR;
	}

	if (FAT32 == fs_info.fat_type)
	{
		status = fat_copy_named_file((unsigned char *)address, (char *)fn);
	}
	else
	{
		ser_putstr("SDMMC card uses an unsupported file system, please use the FAT32 file system\n");
		return SDMMC_ERROR;
	}

	sdmmc_fini(&sdmmc);

	return status;
}

int main()
{
	unsigned image = QNX_LOAD_ADDR;


	init_aips();

	init_clocks();

	init_regulator_settings();

	if ( get_imx6_type() == MX6_CHIP_TYPE_QUAD_OR_DUAL)
	{
		if ( get_imx6_rev() == MX6_CHIP_REV_2_0 )
		{
			apply_mx6_sdl_dcd_values((unsigned long) &dcd_hdr_quadplus);
		}
		else
		{
			apply_mx6_sdl_dcd_values((unsigned long) &dcd_hdr_quad);
		}
		
		init_pinmux_quad_dual();
	}

	/* DL board */
	else if (get_imx6_type() == MX6_CHIP_TYPE_DUAL_LITE_OR_SOLO)
	{
		apply_mx6_sdl_dcd_values((unsigned long) &dcd_hdr_solo);
		init_pinmux_dual_lite_solo();
	}
	else
	{
		apply_mx6_sdl_dcd_values((unsigned long) &dcd_hdr_quad);
		init_pinmux_quad_dual();
	}

	/* Init serial interface */
	init_serial_mx6x();

	ser_putstr("\nWelcome to QNX Neutrino Initial Program Loader for Freescale i.MX6 Quad/Dual/DualLite/Solo/QuadPlus Sabre-ARD RevB (ARM Cortex-A9 MPCore)\n");
	
	if ( get_imx6_type() == MX6_CHIP_TYPE_QUAD_OR_DUAL)
	{
		if ( get_imx6_rev() == MX6_CHIP_REV_2_0)
		{
			ser_putstr("\ni.MX6 Quad Plus CPU detected.\n\n");
		}
		else
		{
			ser_putstr("\ni.MX6 Quad or Dual CPU detected.\n\n");
		}
	}
	else if (get_imx6_type() == MX6_CHIP_TYPE_DUAL_LITE_OR_SOLO)
	{
		ser_putstr("\ni.MX6 Dual Lite CPU detected.\n\n");
	}
	else
	{
		ser_putstr("\ni.MX6 unknown CPU type.\n\n");
		ser_putstr("CHIP ID = ");
		ser_puthex(get_imx6_type());
	}

	char c = 'm'; /* default: load ifs from emmc card */

	while (1) {

		if(!c){
			ser_putstr("Command:\n");
			ser_putstr("Press 'D' for serial download, using the 'sendnto' utility\n");
			ser_putstr("Press 'M' for SDMMC download, IFS filename MUST be 'QNX-IFS'.\n");
			c = ser_getchar();
		}
		switch (c) {
		case 'D':
		case 'd':
			ser_putstr("send image now...\n");
			if (image_download_ser(image)) {
				ser_putstr("download failed...\n");
				continue;
			}
			else
				ser_putstr("download OK...\n");
			break;
		case 'M':
		case 'm':
			ser_putstr("SDMMC download...\n");
			if (sdmmc_load_file(image, "QNX-IFS") == 0) {
				ser_putstr("load image done.\n");
				/* Proceed to image scan */
				break;
			}
			else
			{
				ser_putstr("Load image failed.\n");
				continue;
			}
		default:
			ser_putstr("Unknown command.\n");
			continue;
		}

		image = image_scan(image, image + 0x200);

		if (image != 0xffffffff) {
			ser_putstr("Found image               @ 0x");
			ser_puthex(image);
			ser_putstr("\n");
			image_setup(image);

			ser_putstr("Jumping to startup        @ 0x");
			ser_puthex(startup_hdr.startup_vaddr);
			ser_putstr("\n\n");
			image_start(image);

			/* Never reach here */
			return 0;
		}

		ser_putstr("Image_scan failed...\n");
	}

	return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/ipl/boards/mx6x-sabreARD/main.c $ $Rev: 783570 $")
#endif
