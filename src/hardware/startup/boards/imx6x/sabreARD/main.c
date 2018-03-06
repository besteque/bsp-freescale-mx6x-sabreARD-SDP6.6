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

/*
 * i.MX6X Sabre-AI board with Cortex-A9 MPCore
 */

#include "startup.h"
#include <time.h>
#include <stdbool.h>
#include "board.h"
#include <hw/mini_driver.h>
#include "mini_can.h"

/*
 * Mini driver micros
 * Note: Define/enable one mdriver instance each time
 */
#if 0
#undef INCLUDE_MINI_DATA
#undef INCLUDE_MINI_TIMER
#define INCLUDE_MINI_CAN
#endif

#ifdef INCLUDE_MINI_TIMER
extern int mini_timer(int state, void *data);
#endif

extern int dcache_enable;

const struct callout_slot callouts[] = {
	{ CALLOUT_SLOT( reboot, _mx6x) },
};

const struct debug_device debug_devices[] = {
	{ 	"mx1",
		{"0x02020000^0.115200.80000000.16",
		},
		init_mx1,
		put_mx1,
		{	&display_char_mx1,
			&poll_key_mx1,
			&break_detect_mx1,
		}
	},
};

unsigned mx6x_per_clock = 0;
uint32_t uart_clock;

#ifdef INCLUDE_MINI_CAN
/* Up to 32 individual message IDs can be filtered.
 * To receive all standard IDs:
 *   - set .id to CAN_RX_ALL
 *   - set .is_extended to false
 * To receive all IDs (standard and extended):
 *   - set .id to CAN_RX_ALL
 *   - set .is_extended to true
 * Note: Setting any .id in the list to CAN_RX_ALL will cause all mailboxes to
 *       receive all messages IDs. (standard or standard and extended based on
 *       the .is_extended setting)
 */
static minican_id_t rx_msgs[] = {
	{.id=0x700, .is_extended=false},
	{.id=0x1230, .is_extended=true}
};

/* Up to 32 canned messages can be set on init */
static minican_msg_t tx_init_msgs[] = {
	{.arb={.id=0x503, .is_extended=false}, .len=8, .data={0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}},
	{.arb={.id=0x1200, .is_extended=true}, .len=4, .data={0x88, 0x99, 0xAA, 0xBB}}
};

/* minidriver is expecting this configuration and all lists within it to be
 * either static or global. It stores a pointer to them.
 */
static minican_config_t minican_configuration = {
	.rate=MINICAN_BAUD_50K,
	.msgs_to_rx={.count=(sizeof(rx_msgs)/sizeof(minican_id_t)), .buf=&rx_msgs[0]},
	.init_tx_msgs={.count=(sizeof(tx_init_msgs)/sizeof(minican_msg_t)), .buf=&tx_init_msgs[0]}
};
#endif

/*
 * main()
 *	Startup program executing out of RAM
 *
 * 1. It gathers information about the system and places it in a structure
 *    called the system page. The kernel references this structure to
 *    determine everything it needs to know about the system. This structure
 *    is also available to user programs (read only if protection is on)
 *    via _syspage->.
 *
 * 2. It (optionally) turns on the MMU and starts the next program
 *    in the image file system.
 */
int
main(int argc, char **argv, char **envv)
{
	int		opt, options = 0;
	int     init_can     = 0;
	int     nor_type     = 0;
	int     init_parallel_flash = 0;
	int     init_spi_flash      = 0;
	bool    imx6_quad_dual;
	bool    imx6_dl_solo;
	unsigned long ram_size;

	/*
	 * Initialise debugging output
	 */
	select_debug(debug_devices, sizeof(debug_devices));

	add_callout_array(callouts, sizeof(callouts));

	enable_sdma_copy = FALSE;

	/* identify the imx6 processor we're running on */
	imx6_quad_dual = (get_mx6_chip_type()==MX6_CHIP_TYPE_QUAD_OR_DUAL) ? TRUE:FALSE;
	imx6_dl_solo   = (get_mx6_chip_type()==MX6_CHIP_TYPE_DUAL_LITE_OR_SOLO) ? TRUE:FALSE;

	// common options that should be avoided are:
	// "AD:F:f:I:i:K:M:N:o:P:R:S:Tvr:j:Z"
	while ((opt = getopt(argc, argv, COMMON_OPTIONS_STRING "mWcn:s")) != -1) {

		switch (opt) {
			case 'm':
				dcache_enable = TRUE;
				break;
			case 'W':
				options |= MX6X_WDOG_ENABLE;
				break;
			case 'c':
				init_can = 1;
				break;
			case 'n':
				nor_type = strtoul(optarg, 0, 0);

				switch(nor_type) {
				case 0: // No flash - allows I2C3 to be used
					init_parallel_flash = 0;
					init_spi_flash = 0;
					break;

				case 1: // Parallel NOR Flash
					init_parallel_flash = 1;
					init_spi_flash = 0;
					break;

				case 2: // SPI NOR Flash
					init_parallel_flash = 0;
					init_spi_flash = 1;
					break;

				}
				break;
			case 's':
				enable_sdma_copy = TRUE;
				break;

			default:
				handle_common_option(opt);
				break;
		}
	}

	if (options &  MX6X_WDOG_ENABLE) {
		/*
		* Enable WDT
		*/
		mx6x_wdg_reload();
		mx6x_wdg_enable();
	}

	/*
	 * Collect information on all free RAM in the system
	 * Note:  an imx6 DL will only be configured to use 1GB DDR3
	 *        although it can use 2GB using 64-bit.  This is
	 *        because we can't tell if this is a Solo or DL.
	 */
	if ( imx6_quad_dual )
	{
		ram_size = MX6S_SDRAM_SIZE;//MX6Q_D_DL_SDRAM_SIZE;
	} else {
		ram_size = MX6S_SDRAM_SIZE;
	}
	mx6x_init_raminfo(ram_size);

	/*
	 * set CPU frequency, currently max stable CPU freq is 792MHz
	 */
	if (cpu_freq == 0)
		cpu_freq = mx6x_get_cpu_clk();

	/*
	 * Remove RAM used by modules in the image
	 */
	alloc_ram(shdr->ram_paddr, shdr->ram_size, 1);

	/*
	 * Initialize SMP
	 */
	init_smp();

	/*
	 * Initialize MMU
	 */
	if (shdr->flags1 & STARTUP_HDR_FLAGS1_VIRTUAL)
		init_mmu();

	/* Initialize the Interrupts related Information */
	init_intrinfo();

	/* Initialize the Timer related information */
	mx6x_init_qtime();

	/* Init Clocks (must happen after timer is initialized */
	mx6x_init_clocks();

	/* Init L2 Cache Controller */
	init_cacheattr();

#ifdef INCLUDE_MINI_DATA
    {
        /* allocate 64k of ram for the minidrivers use */
        paddr_t  mdriver_addr_data = alloc_ram(~0L,  MINI_DATA_BUF_SIZE, 1);
        mdriver_add(MINI_DATA_NAME, 0, mini_data,  mdriver_addr_data,  MINI_DATA_BUF_SIZE);
    }
#endif

	/* Initialize the CPU related information */
	init_cpuinfo();

	/* Initialize the Hwinfo section of the Syspage */
	init_hwinfo();

	add_typed_string(_CS_MACHINE, "i.MX6Q Sabre-ARD Board");

	/* AIPSTZ init */
	mx6x_init_aipstz();

	/*
	 * init UART 1 to print debug info
	 */
	mx6sdl_init_uart_1();

	/* Configure PIN MUX to enable i2c */
	mx6sdl_init_i2c();

	/* Init USB OTG, dev mode */
	mx6sdl_usb_otg_host_init();
	kprintf("[xy]mx6sdl_usb_otg_host_init NOK\n");

	/* Enable CPU Card peripherals such as Ethernet PHY */
	//init_cpucard_reset_steer();
	//kprintf("[xy]init_cpucard_reset_steer NOK\n");

	/* Enable Mainboard peripherals such as ADV7180 decoder IC */
	//init_mainboard_reset_steer();
	//kprintf("[xy]init_mainboard_reset_steer NOK\n");

	/* usb host mode */
	mx6x_usb_host1_init();
	kprintf("[xy]mx6x_usb_host1_init OK\n");

	if (init_parallel_flash) /* Parallel NOR flash */
	{
		/* Configure PIN MUX and steering to enable parallel NOR flash */
		mx6x_nor_init();
	}
	else if (init_spi_flash) /* SPI NOR flash */
	{
		/* Configure PIN MUX and steering to enable SPI NOR flash */
		if ( imx6_quad_dual )
		{
			mx6q_init_ecspi();
		}
		else if ( imx6_dl_solo )
		{
			mx6sdl_init_ecspi();
		}
	}
	/*
	 * If not using SPI or parallel NOR then I2C3 and all devices on the I2C3 bus
	 * can be used, including the ADV7180 (for video capture)
	 */
	else
	{
		//init_video_steer();
		//kprintf("[xy]init_video_steer NOK\n");
	}

	/* pin KEY_COL2 is being shared between CAN and ENET */
	if (init_can)
	{
		/* Init CAN Controller */
		if ( imx6_quad_dual )
		{
			mx6q_init_can();
		}
		else if ( imx6_dl_solo )
		{
			mx6sdl_init_can();
		}
		init_enet_can_steer();
	}
	else
	{
		/* Init ENET Controller */
		mx6x_init_mac();

		/* ENET pin muxing differences */
		if ( imx6_quad_dual )
		{
			mx6q_init_enet();
		}
		else if ( imx6_dl_solo )
		{
			mx6sdl_init_enet();
			kprintf("[xy]mx6sdl_init_enet ok\n");
		}
	}

#ifdef INCLUDE_MINI_CAN
	{
		mini_can_preconfigure(&minican_configuration);
		/* allocate 64k of ram for the CAN minidriver to use */
		paddr_t mdriver_addr_can = alloc_ram(NULL_PADDR,  MINI_DATA_BUF_SIZE, 1);
		mdriver_add(MINI_CAN_NAME, MX6X_CAN1_IRQ, mini_can, mdriver_addr_can, MINI_DATA_BUF_SIZE);
	}
#endif

	/* Test if LDO_PU is gated (responsible for VPU/GPU power domain) */
	if (pmu_get_voltage(LDO_PU) == CPU_POWER_GATED_OFF_VOLTAGE)
	{
		pmu_power_up_gpu();
		kprintf("[xy]pmu_power_up_gpu ok\n");
	}

	/* init PMIC */
	mx6sdl_init_pmic();
	kprintf("[xy]mx6sdl_init_pmic ok\n");

	/* init UART 2 */
	//mx6sdl_init_uart_2();

	/* Configure PIN MUX to enable SD */
	mx6sdl_init_usdhc();
	kprintf("[xy]mx6sdl_init_usdhc ok\n");

	/* Configure pins for LVDS, LCD display*/
	mx6x_init_displays();

	/* Set GPU3D clocks */
	mx6x_init_gpu3D();

	/* Configure esai pins */
	//mx6sdl_init_esai();

	/* Configure spdif pins */
	//mx6sdl_init_spdif();
	//kprintf("[xy]mx6sdl_init_spdif ok\n");

	/*
	 * If using an LVDS display mx6x_init_lvds_clock() should be called which will select
	 * PLL5 as the LDB clock source.
	 */
	mx6x_init_lvds_clock();
	kprintf("[xy]mx6x_init_lvds_clock ok\n");

	pmu_set_standard_ldo_voltages();

	mx6x_dump_clocks();
#ifdef INCLUDE_MINI_TIMER
    {
      /* allocate 64k of ram for the minidrivers use */
        paddr_t mdriver_addr_timer = alloc_ram(~0L,  MINI_DATA_BUF_SIZE, 1);
        mdriver_add(MINI_TIMER_NAME, MX6X_EPIT2_IRQ, mini_timer, mdriver_addr_timer, MINI_DATA_BUF_SIZE);
    }
#endif

	/*
	 * Load bootstrap executables in the image file system and Initialise
	 * various syspage pointers. This must be the _last_ initialisation done
	 * before transferring control to the next program.
	 */
	init_system_private();

	/*
	 * This is handy for debugging a new version of the startup program.
	 * Commenting this line out will save a great deal of code.
	 */
	print_syspage();
	return 0;
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/sabreARD/main.c $ $Rev: 790154 $")
#endif
