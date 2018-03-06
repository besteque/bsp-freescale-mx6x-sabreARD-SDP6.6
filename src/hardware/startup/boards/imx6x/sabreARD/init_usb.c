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

/*
 * The i.MX6 series SOCs contain 4 USB controllers:
 * One OTG controller which can function in Host or Device mode.  The OTG module uses an on chip UTMI PHY
 * One Host controller which uses an on chip UTMI phy
 * Two Host controllers which use on chip HS-IC PHYs
 */

/*
 * The i.MX6 Q Sabre-ARD BSP currently supports the OTG controller.
 */

#include "startup.h"
#include "board.h"
#include "mx6x_i2c.h"

void init_otg_vbus(void)
{
	mx6x_i2c_dev_t vbus_dev = {
			MX6X_I2C3_BASE,
			I2C_CLK_DIV,
			IO_SLAVE_ADDR_C
			};

	uint8_t val;

	init_i2c_bus(&vbus_dev);

	i2c_read(&vbus_dev, OUT_PORT_REG, &val);
	mx6x_usleep(1000);
	val |= OUT_PORT_1;

	i2c_write(&vbus_dev, OUT_PORT_REG, &val);
	mx6x_usleep(1000);

	i2c_read(&vbus_dev, CONF_REG, &val);
	mx6x_usleep(1000);

	val &= ~CONF_REG_1;
	i2c_write(&vbus_dev, CONF_REG, &val);
	mx6x_usleep(1000);
}

void mx6q_usb_otg_host_init(void)
{

	/* Turning on the USB_OTG_VBUS */
	init_otg_vbus();

	/* ID pin muxing */
	/* USB OTG_ID Input */
	pinmux_set_swmux(SWMUX_ENET_RX_ER, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_ENET_RX_ER, MX6X_PAD_SETTINGS_USB);

	/* setup GPIO2[8] as USB_CLK_EN_B line */
	pinmux_set_swmux(SWMUX_SD4_DAT0, MUX_CTL_MUX_MODE_ALT5);

	/* USB OTG select ENET_RX_ER */
	out32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1, in32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1) & ~(1 << 13));

	/* Initialize OTG core */
	mx6x_init_usb_otg();

	/* OTG Host connects to PHY0  */
	mx6x_init_usb_phy(MX6X_USBPHY0_BASE);
}

void mx6sdl_usb_otg_host_init(void)
{

	/* Turning on the USB_OTG_VBUS */
	init_otg_vbus();

	/* ID pin muxing */
	/* USB OTG_ID Input */
	pinmux_set_swmux(SWMUX_SDL_ENET_RX_ER, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_ENET_RX_ER, MX6X_PAD_SETTINGS_USB);
	pinmux_set_input(SWINPUT_SDL_ANATOP_USB_OTG_ID, 0x0);

	/* setup GPIO2[8] as USB_CLK_EN_B line */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT0, MUX_CTL_MUX_MODE_ALT5);

	/* USB OTG select ENET_RX_ER */
	out32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1, in32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1) & ~(1 << 13));

	/* Initialize OTG core */
	mx6x_init_usb_otg();

	/* OTG Host connects to PHY0  */
	mx6x_init_usb_phy(MX6X_USBPHY0_BASE);
}

void mx6x_usb_host1_init(void)
{
	/*USB HOST1 on base board */
	/* USB_HOST1_OC_B*/
//	pinmux_set_swmux(SWMUX_EIM_WAIT, MUX_CTL_MUX_MODE_ALT5);

	mx6x_init_usb_host1();

	/* USB Host1 connects to PHY1  */
	mx6x_init_usb_phy(MX6X_USBPHY1_BASE);

}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/sabreARD/init_usb.c $ $Rev: 703188 $")
#endif
