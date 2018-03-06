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

#include "startup.h"
#include "board.h"
#include "mx6x_i2c.h"

/*
 * Name:	max7310_set_output
 * Description:	This function configures a MAX7310 port expander port as an input or an output
 * Parameters:
 * 		i2c_base - Base address (physical address) of I2C controller, e.g. MX6X_I2C3_BASE
 *		slave_addr - I2C address of MAX7310 device, e.g. 0x30
 *		output_port - bit shifted value indicating output port, e.g. OUT_PORT_3
 		output_value - Output value of GPIO - 0x0 or 0x1
 */
void max7310_set_output(uint32_t i2c_base, unsigned slave_addr, uint32_t output_port, uint32_t output_value)
{
	mx6x_i2c_dev_t max7310_dev = {
		i2c_base,
		I2C_CLK_DIV,
		slave_addr
	};

	uint8_t val;
	if (output_port > 0xff)
	{
		kprintf("%s: output port %d invalid\n", __FUNCTION__, output_port);
		return;
	}

	init_i2c_bus(&max7310_dev);

	/* Set output value to high or low */
	i2c_read(&max7310_dev, OUT_PORT_REG, &val);
	mx6x_usleep(1000);

	if (output_value)
		val |= output_port;
	else
		val &= ~output_port;

	i2c_write(&max7310_dev, OUT_PORT_REG, &val);
	mx6x_usleep(1000);

	/* Configure port as an output */
	i2c_read(&max7310_dev, CONF_REG, &val);
	mx6x_usleep(1000);

	val &= ~output_port;
	i2c_write(&max7310_dev, CONF_REG, &val);
	mx6x_usleep(1000);

	return;
}

/*
 * The default mode of the Ethernet and CAN data steering is Ethernet.
 * Therefore, for using CAN, the ENET_CAN1_STEER needs to steer to CAN1_TXCAN
 */
void init_enet_can_steer(void)
{
	max7310_set_output(MX6X_I2C3_BASE, IO_SLAVE_ADDR_B, OUT_PORT_3, 1);
}

void init_nor_steer(void)
{
	/*
	 * set EIMD18_I2C3_STEER and EIMD30_BTUART3_STEER(PORT_EXP_B4) to be zero
	 */
	max7310_set_output(MX6X_I2C3_BASE, IO_SLAVE_ADDR_B, OUT_PORT_4, 0);

	/* Enable EIM_D18 route path - disable I2C3 */
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_DR) & ~(1<<4));
}

/*
 * Bring CPU_PER_RST_B (PORT_EXP_A2) out of reset which will bring various CPU card peripherals
 * such as the Ethernet PHY, devices connected to PCIe port and devices connected to MIPI port out of reset.
 */
void init_cpucard_reset_steer(void)
{

	max7310_set_output(MX6X_I2C3_BASE, IO_SLAVE_ADDR_A, OUT_PORT_2, 1);
}

/*
 * Bring MAIN_PER_RST_B (PORT_EXP_A3) out of reset which will bring various main board peripherals
 * out of reset such as the ADV7180 decoder IC.
 */
void init_mainboard_reset_steer(void)
{
	max7310_set_output(MX6X_I2C3_BASE, IO_SLAVE_ADDR_A, OUT_PORT_3, 1);
}

/* Power up ADV7180 decoder IC */
void init_video_steer(void)
{
	max7310_set_output(MX6X_I2C3_BASE, IO_SLAVE_ADDR_B, OUT_PORT_2, 1);
}


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/sabreARD/steering.c $ $Rev: 751697 $")
#endif
