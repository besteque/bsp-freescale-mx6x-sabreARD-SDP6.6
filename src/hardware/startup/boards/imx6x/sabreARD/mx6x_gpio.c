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
 * Routines to initialize the various hardware subsystems
 * on the i.MX6Q Sabre-ARD
 */

#include "startup.h"
#include "board.h"

#define MX6Q_PAD_SETTINGS_USDHC3_WP	(PAD_CTL_PKE_ENABLE | PAD_CTL_PUE_PULL | PAD_CTL_PUS_22K_PU | PAD_CTL_SPEED_LOW | \
							PAD_CTL_DSE_80_OHM | PAD_CTL_HYS_ENABLE)

#define MX6_PAD_SETTINGS_IPU (PAD_CTL_HYS_ENABLE | PAD_CTL_PKE_ENABLE | \
								PAD_CTL_PUS_100K_PU | PAD_CTL_SPEED_MEDIUM | \
								PAD_CTL_DSE_40_OHM | PAD_CTL_SRE_SLOW | \
							   PAD_CTL_ODE_DISABLE | PAD_CTL_PUE_PULL)


#define CCM_CCOSR_CLKO1_EN	(0x1 << 7)
#define CCM_CCOSR_CLKO1_DIV_8	(0x7 << 4)
#define CCM_CCOSR_CLKO1_AHB	(0xb << 0)

#define MX6_INTR_CFG_FALLING 0x3
#define GPIO5_16_ICR2_SHIFT     0

#define MX6DQ_GPR1_MIPI_IPU1_MUX_MASK	(0x1 << 19)
#define MX6DQ_GPR1_MIPI_IPU1_PARALLEL	(0x1 << 19)

#define MX6SDL_GPR13_IPU_CSI1_MUX_MASK	(0x7 << 3)
#define MX6SDL_GPR13_IPU_CSI1_MIPI_CSI1	(0x1 << 3)
#define MX6SDL_GPR13_IPU_CSI0_MUX_MASK	(0x7)
#define MX6SDL_GPR13_IPU_CSI0_IPU_CSI0	(0x4)
void mx6q_init_i2c2(void)
{
	/* I2C2  SCL */
	pinmux_set_swmux(SWMUX_EIM_EB2, MUX_CTL_MUX_MODE_ALT6 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_EIM_EB2, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_I2C2_IPP_SCL_IN, 0x0);

	/* I2C2  SDA */
	pinmux_set_swmux(SWMUX_KEY_ROW3, MUX_CTL_MUX_MODE_ALT4 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_KEY_ROW3, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_I2C2_IPP_SDA_IN, 0x1);
}

void mx6sdl_init_i2c(void)
{
	/* I2C1  SDA */
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT8, MUX_CTL_MUX_MODE_ALT4 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_CSI0_DAT8, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_SDL_I2C1_IPP_SDA_IN, 0);

	/* I2C1  SCL */
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT9, MUX_CTL_MUX_MODE_ALT4 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_CSI0_DAT9, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_SDL_I2C1_IPP_SCL_IN, 0);


	/* I2C2 SCL */
	pinmux_set_swmux(SWMUX_SDL_KEY_COL3, MUX_CTL_MUX_MODE_ALT4 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_KEY_COL3, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_I2C2_IPP_SCL_IN, 1);

	/* I2C2  SDA */
	pinmux_set_swmux(SWMUX_SDL_KEY_ROW3, MUX_CTL_MUX_MODE_ALT4 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_KEY_ROW3, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_I2C2_IPP_SDA_IN, 1);


	/* I2C3 SCL */
	pinmux_set_swmux(SWMUX_SDL_GPIO_3, MUX_CTL_MUX_MODE_ALT2 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_GPIO_3, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_I2C3_IPP_SCL_IN, 0x1);

	/* I2C3  SDA */
	pinmux_set_swmux(SWMUX_SDL_GPIO_6, MUX_CTL_MUX_MODE_ALT2 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_GPIO_6, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_I2C3_IPP_SDA_IN, 0x2);
}

void mx6sdl_init_i2c2(void)
{
	/* I2C2  SCL */
	pinmux_set_swmux(SWMUX_SDL_EIM_EB2, MUX_CTL_MUX_MODE_ALT6 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_EIM_EB2, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_SDL_I2C2_IPP_SCL_IN, 0x0);

	/* I2C2  SDA */
	pinmux_set_swmux(SWMUX_SDL_KEY_ROW3, MUX_CTL_MUX_MODE_ALT4 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_KEY_ROW3, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_SDL_I2C2_IPP_SDA_IN, 0x1);
}

void mx6q_init_i2c3(void)
{
	/* I2C3 SCL */
	pinmux_set_swmux(SWMUX_GPIO_3, MUX_CTL_MUX_MODE_ALT2 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_GPIO_3, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_I2C3_IPP_SCL_IN, 1);

	/* I2C3  SDA */
	pinmux_set_swmux(SWMUX_EIM_D18, MUX_CTL_MUX_MODE_ALT6 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_EIM_D18, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_I2C3_IPP_SDA_IN, 0);

	/* EIM_A24__GPIO_5_4 steer logic enable */
	pinmux_set_swmux(SWMUX_EIM_A24, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) | (1<<4)); /* set as an output */
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_DR) | (1<<4));  /* Enable I2C3 SDA route path */

	/* GPIO1[15] */
	pinmux_set_swmux(SWMUX_SD2_DAT0, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) | (1<<15)); //set as an output
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) & ~(1<<15)); // low
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) | (1<<15)); // high
}

void mx6sdl_init_i2c3(void)
{
	/* I2C3 SCL */
	pinmux_set_swmux(SWMUX_SDL_GPIO_3, MUX_CTL_MUX_MODE_ALT2 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_GPIO_3, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_SDL_I2C3_IPP_SCL_IN, 1);

	/* I2C3  SDA */
	pinmux_set_swmux(SWMUX_SDL_EIM_D18, MUX_CTL_MUX_MODE_ALT6 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D18, MX6X_PAD_SETTINGS_I2C);
	pinmux_set_input(SWINPUT_SDL_I2C3_IPP_SDA_IN, 0);

	/* EIM_A24__GPIO_5_4: I2C3 steering logic enable */
	pinmux_set_swmux(SWMUX_SDL_EIM_A24, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) | (1<<4)); /* set as an output */
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_DR) | (1<<4));  /* Enable I2C3 SDA route path */

	/*
	 * GPIO1[15]: I2C port expander reset (active low)
	 *            I2C port expander controls the USB OTG Power On, Sat reset and NAND BT Wifi steer
	 *            Toggle the port expander reset signal
	 */
	pinmux_set_swmux(SWMUX_SDL_SD2_DAT0, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) | (1<<15)); //set as an output
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) & ~(1<<15)); // low
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) | (1<<15)); // high
}

void mx6q_init_enet(void)
{
	uint32_t reg;

	// RGMII MDIO - transfers control info between MAC and PHY
	pinmux_set_swmux(SWMUX_KEY_COL1, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_input(SWINPUT_ENET_IPP_IND_MAC0_MDIO,1);

	// RGMII MDC - output from MAC to PHY, provides clock reference for MDIO
	pinmux_set_swmux(SWMUX_KEY_COL2, MUX_CTL_MUX_MODE_ALT4);

	// RGMII TXC - output from MAC, provides clock used by RGMII_TXD[3:0], RGMII_TX_CTL
	pinmux_set_swmux(SWMUX_RGMII_TXC, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_TXC, MX6X_PAD_SETTINGS_ENET);

	// RGMII TXD[3:0] - Transmit Data Output
	pinmux_set_swmux(SWMUX_RGMII_TD0, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_TD0, MX6X_PAD_SETTINGS_ENET);

	pinmux_set_swmux(SWMUX_RGMII_TD1, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_TD1, MX6X_PAD_SETTINGS_ENET);

	pinmux_set_swmux(SWMUX_RGMII_TD2, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_TD2, MX6X_PAD_SETTINGS_ENET);

	pinmux_set_swmux(SWMUX_RGMII_TD3, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_TD3, MX6X_PAD_SETTINGS_ENET);

	// RGMII TX_CTL - contains TXEN on TXC rising edge, TXEN XOR TXERR on TXC falling edge
	pinmux_set_swmux(SWMUX_RGMII_TX_CTL, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_TX_CTL, MX6X_PAD_SETTINGS_ENET);

	// set ENET_REF_CLK to mux mode 1 - TX_CLK, this is a 125MHz input which is driven by the PHY
	pinmux_set_swmux(SWMUX_ENET_REF_CLK, MUX_CTL_MUX_MODE_ALT1);

	pinmux_set_swmux(SWMUX_RGMII_RXC, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_RXC, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_ENET_IPP_IND_MAC0_RXCLK,0);

	pinmux_set_swmux(SWMUX_RGMII_RD0, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_RD0, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_ENET_IPP_IND_MAC_RXDATA_0,0);

	pinmux_set_swmux(SWMUX_RGMII_RD1, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_RD1, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_ENET_IPP_IND_MAC_RXDATA_1,0);

	pinmux_set_swmux(SWMUX_RGMII_RD2, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_RD2, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_ENET_IPP_IND_MAC_RXDATA_2,0);

	pinmux_set_swmux(SWMUX_RGMII_RD3, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_RD3, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_ENET_IPP_IND_MAC_RXDATA_3,0);

	pinmux_set_swmux(SWMUX_RGMII_RX_CTL, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_RGMII_RX_CTL, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_ENET_IPP_IND_MAC0_RXEN,0);

	/* RGMII Phy interrupt set to GPIO4[5] */
	pinmux_set_swmux(SWMUX_GPIO_19, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO4_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO4_BASE + MX6X_GPIO_GDIR) & ~(1<<5));

   /* set up GPIO_16 for timestamps */
	pinmux_set_swmux(SWMUX_GPIO_16, MUX_CTL_MUX_MODE_ALT2 | MUX_CTL_SION);
	pinmux_set_input(SWINPUT_ENET_IPG_CLK_RMII, 1);

	/* set ENET_REF_CLK to PLL (not in RevC docs, new definition */
	reg = in32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1);
	reg |= (1<<21);
	out32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1, reg);

	pinmux_set_swmux(SWMUX_GPIO_0, MUX_CTL_MUX_MODE_ALT0);
}

void mx6sdl_init_enet(void)
{
	uint32_t reg;

	// RGMII MDIO - transfers control info between MAC and PHY
	pinmux_set_swmux(SWMUX_SDL_KEY_COL1, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_input(SWINPUT_SDL_ENET_IPP_IND_MAC0_MDIO,1);

	// RGMII MDC - output from MAC to PHY, provides clock reference for MDIO
	pinmux_set_swmux(SWMUX_SDL_KEY_COL2, MUX_CTL_MUX_MODE_ALT4);

	// RGMII TXC - output from MAC, provides clock used by RGMII_TXD[3:0], RGMII_TX_CTL
	pinmux_set_swmux(SWMUX_SDL_RGMII_TXC, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_TXC, MX6X_PAD_SETTINGS_ENET);

	// RGMII TXD[3:0] - Transmit Data Output
	pinmux_set_swmux(SWMUX_SDL_RGMII_TD0, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_TD0, MX6X_PAD_SETTINGS_ENET);

	pinmux_set_swmux(SWMUX_SDL_RGMII_TD1, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_TD1, MX6X_PAD_SETTINGS_ENET);

	pinmux_set_swmux(SWMUX_SDL_RGMII_TD2, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_TD2, MX6X_PAD_SETTINGS_ENET);

	pinmux_set_swmux(SWMUX_SDL_RGMII_TD3, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_TD3, MX6X_PAD_SETTINGS_ENET);

	// RGMII TX_CTL - contains TXEN on TXC rising edge, TXEN XOR TXERR on TXC falling edge
	pinmux_set_swmux(SWMUX_SDL_RGMII_TX_CTL, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_TX_CTL, MX6X_PAD_SETTINGS_ENET);

	// set ENET_REF_CLK to mux mode 1 - TX_CLK, this is a 125MHz input which is driven by the PHY
	pinmux_set_swmux(SWMUX_SDL_ENET_REF_CLK, MUX_CTL_MUX_MODE_ALT1);

	// RXC input mode is GPIO
	pinmux_set_swmux(SWMUX_SDL_RGMII_RXC, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_RXC, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_SDL_ENET_IPP_IND_MAC0_RXCLK,1);   // Solo change

	// RGMII RXD[3:0] - Receive Data inputs
	pinmux_set_swmux(SWMUX_SDL_RGMII_RD0, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_RD0, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_SDL_ENET_IPP_IND_MAC_RXDATA_0,1); // Solo change

	pinmux_set_swmux(SWMUX_SDL_RGMII_RD1, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_RD1, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_SDL_ENET_IPP_IND_MAC_RXDATA_1,1); // Solo change

	pinmux_set_swmux(SWMUX_SDL_RGMII_RD2, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_RD2, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_SDL_ENET_IPP_IND_MAC_RXDATA_2,1); // Solo change

	pinmux_set_swmux(SWMUX_SDL_RGMII_RD3, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_RD3, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_SDL_ENET_IPP_IND_MAC_RXDATA_3,1); // Solo change

	pinmux_set_swmux(SWMUX_SDL_RGMII_RX_CTL, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_RGMII_RX_CTL, MX6X_PAD_SETTINGS_ENET);
	pinmux_set_input(SWINPUT_SDL_ENET_IPP_IND_MAC0_RXEN,1);    // Solo change

	/* RGMII Phy interrupt set to GPIO4[5] */
	pinmux_set_swmux(SWMUX_SDL_GPIO_19, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO4_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO4_BASE + MX6X_GPIO_GDIR) & ~(1<<5));

   /* set up GPIO_16 for timestamps */
	pinmux_set_swmux(SWMUX_SDL_GPIO_16, MUX_CTL_MUX_MODE_ALT2 | MUX_CTL_SION);
	pinmux_set_input(SWINPUT_SDL_ENET_IPG_CLK_RMII, 0); // Solo change

	/* set ENET_REF_CLK to PLL (not in RevC docs, new definition) */
	reg = in32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1);
	reg |= (1<<21);
	out32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1, reg);

	// CLKO output
	pinmux_set_swmux(SWMUX_SDL_GPIO_0, MUX_CTL_MUX_MODE_ALT0);
}

void mx6q_init_usdhc(void)
{
	/***********
	 * SD1
	 ***********/

	/* SD1 CLK */
	pinmux_set_swmux(SWMUX_SD1_CLK, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD1_CLK, MX6X_PAD_SETTINGS_USDHC);

	/* SD1 CMD */
	pinmux_set_swmux(SWMUX_SD1_CMD, MUX_CTL_MUX_MODE_ALT0 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SD1_CMD, MX6X_PAD_SETTINGS_USDHC);

	/* SD1 DAT0 */
	pinmux_set_swmux(SWMUX_SD1_DAT0, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD1_DAT0, MX6X_PAD_SETTINGS_USDHC);

	/* SD1 DAT1 */
	pinmux_set_swmux(SWMUX_SD1_DAT1, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD1_DAT1, MX6X_PAD_SETTINGS_USDHC);

	/* SD1 DAT2 */
	pinmux_set_swmux(SWMUX_SD1_DAT2, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD1_DAT2, MX6X_PAD_SETTINGS_USDHC);

	/* SD1 DAT3 */
	pinmux_set_swmux(SWMUX_SD1_DAT3, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD1_DAT3, MX6X_PAD_SETTINGS_USDHC);

	/* SD1 Card Detect - configure GPIO1[1] as an input */
	pinmux_set_swmux(SWMUX_GPIO_1, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) & ~(1<<1));

	/* SD1 Write Protect - configure GPIO5[20] as an input */
	pinmux_set_swmux(SWMUX_CSI0_DATA_EN, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) & ~(1<<20));

	/***********
	 * SD3
	 ***********/

	/* SD3 CLK */
	pinmux_set_swmux(SWMUX_SD3_CLK, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD3_CLK, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 CMD */
	pinmux_set_swmux(SWMUX_SD3_CMD, MUX_CTL_MUX_MODE_ALT0 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SD3_CMD, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 DAT0 */
	pinmux_set_swmux(SWMUX_SD3_DAT0, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD3_DAT0, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 DAT1 */
	pinmux_set_swmux(SWMUX_SD3_DAT1, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD3_DAT1, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 DAT2 */
	pinmux_set_swmux(SWMUX_SD3_DAT2, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD3_DAT2, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 DAT3 */
	pinmux_set_swmux(SWMUX_SD3_DAT3, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD3_DAT3, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 DAT4 */
	pinmux_set_swmux(SWMUX_SD3_DAT4, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD3_DAT4, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 DAT5 */
	pinmux_set_swmux(SWMUX_SD3_DAT5, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD3_DAT5, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 DAT6 */
	pinmux_set_swmux(SWMUX_SD3_DAT6, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD3_DAT6, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 DAT7 */
	pinmux_set_swmux(SWMUX_SD3_DAT7, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SD3_DAT7, MX6X_PAD_SETTINGS_USDHC);

	/* SD3 Write Protect - configure GPIO1[13] as an input */
	pinmux_set_swmux(SWMUX_SD2_DAT2, MUX_CTL_MUX_MODE_ALT5);
//	pinmux_set_padcfg(SWPAD_SD2_DAT2, MX6Q_PAD_SETTINGS_USDHC_WP);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) & ~(1<<13));

	/* SD3 Card Detect - configure GPIO6[15] as an input */
	pinmux_set_swmux(SWMUX_NANDF_CS2, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO6_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO6_BASE + MX6X_GPIO_GDIR) & ~(1<<15));
}

void mx6sdl_init_usdhc(void)
{
	/***********
	 * SD4
	 ***********/

	/* SD4 CLK */
	pinmux_set_swmux(SWMUX_SDL_SD4_CLK, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_SD4_CLK, MX6X_PAD_SETTINGS_USDHC);

	/* SD4 CMD */
	pinmux_set_swmux(SWMUX_SDL_SD4_CMD, MUX_CTL_MUX_MODE_ALT0 | MUX_CTL_SION);
	pinmux_set_padcfg(SWPAD_SDL_SD4_CMD, MX6X_PAD_SETTINGS_USDHC);

	/* SD4 DAT0 */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT0, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT0, MX6X_PAD_SETTINGS_USDHC);

	/* SD4 DAT1 */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT1, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT1, MX6X_PAD_SETTINGS_USDHC);

	/* SD4 DAT2 */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT2, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT2, MX6X_PAD_SETTINGS_USDHC);

	/* SD4 DAT3 */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT3, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT3, MX6X_PAD_SETTINGS_USDHC);

	/* SD4 DAT4 */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT4, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT4, MX6X_PAD_SETTINGS_USDHC);

	/* SD4 DAT5 */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT5, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT5, MX6X_PAD_SETTINGS_USDHC);

	/* SD4 DAT6 */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT6, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT6, MX6X_PAD_SETTINGS_USDHC);

	/* SD4 DAT7 */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT7, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT7, MX6X_PAD_SETTINGS_USDHC);

#if 0
	/* SD3 Write Protect - configure GPIO1[13] as an input */
	pinmux_set_swmux(SWMUX_SDL_SD2_DAT2, MUX_CTL_MUX_MODE_ALT5);
//	pinmux_set_padcfg(SWPAD_SDL_SD2_DAT2, MX6Q_PAD_SETTINGS_USDHC_WP);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_GDIR) & ~(1<<13));

	/* SD3 Card Detect - configure GPIO6[15] as an input */
	pinmux_set_swmux(SWMUX_SDL_NANDF_CS2, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO6_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO6_BASE + MX6X_GPIO_GDIR) & ~(1<<15));
#endif

}

void mx6q_init_ecspi(void)
{
	/* SPI SCLK */
	pinmux_set_swmux(SWMUX_EIM_D16, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_EIM_D16, MX6X_PAD_SETTINGS_ECSPI);
	pinmux_set_input(SWINPUT_ECSPI1_IPP_CSPI_CLK, 0x0);

	/* SPI MISO */
	pinmux_set_swmux(SWMUX_EIM_D17, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_EIM_D17, MX6X_PAD_SETTINGS_ECSPI);
	pinmux_set_input(SWINPUT_ECSPI1_IPP_IND_MISO, 0x0);

	/* SPI MOSI */
	pinmux_set_swmux(SWMUX_EIM_D18, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_EIM_D18, MX6X_PAD_SETTINGS_ECSPI);
	pinmux_set_input(SWINPUT_ECSPI1_IPP_IND_MOSI, 0x0);

	/* Select mux mode ALT1 for SS1 */
	pinmux_set_swmux(SWMUX_EIM_D19, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_EIM_D19, MX6X_PAD_SETTINGS_ECSPI);
	pinmux_set_input(SWINPUT_ECSPI1_IPP_IND_SS_B_1, 0x0);

	/* Steer EIM_D18 signal to NOR flash (disables I2C3 SDA) */
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_DR) & ~(1<<4));
}

void mx6sdl_init_ecspi(void)
{
	/* SPI SCLK */
	pinmux_set_swmux(SWMUX_SDL_EIM_D16, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D16, MX6X_PAD_SETTINGS_ECSPI);
	pinmux_set_input(SWINPUT_SDL_ECSPI1_IPP_CSPI_CLK, 0x2); // Solo change

	/* SPI MISO */
	pinmux_set_swmux(SWMUX_SDL_EIM_D17, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D17, MX6X_PAD_SETTINGS_ECSPI);
	pinmux_set_input(SWINPUT_SDL_ECSPI1_IPP_IND_MISO, 0x2); // Solo change

	/* SPI MOSI */
	pinmux_set_swmux(SWMUX_SDL_EIM_D18, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D18, MX6X_PAD_SETTINGS_ECSPI);
	pinmux_set_input(SWINPUT_SDL_ECSPI1_IPP_IND_MOSI, 0x2); // Solo change

	/* Select mux mode ALT1 for SS1 */
	pinmux_set_swmux(SWMUX_SDL_EIM_D19, MUX_CTL_MUX_MODE_ALT1);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D19, MX6X_PAD_SETTINGS_ECSPI);
	pinmux_set_input(SWINPUT_SDL_ECSPI1_IPP_IND_SS_B_1, 0x1); // Solo change

	/* Steer EIM_D18 signal to NOR flash (disables I2C3 SDA) */
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_DR) & ~(1<<4));
}

void mx6q_init_spdif(void)
{
	/* SPDIF_IN1 */
	pinmux_set_swmux(SWMUX_KEY_COL3, MUX_CTL_MUX_MODE_ALT6);
	pinmux_set_padcfg(SWPAD_KEY_COL3, MX6X_PAD_SETTINGS_SPDIF);
	pinmux_set_input(SWINPUT_SPDIF_SPDIF_IN1, 0x2);
}

void mx6sdl_init_spdif(void)
{
	/* SPDIF_IN1 */
	pinmux_set_swmux(SWMUX_SDL_KEY_COL3, MUX_CTL_MUX_MODE_ALT6);
	pinmux_set_padcfg(SWPAD_SDL_KEY_COL3, MX6X_PAD_SETTINGS_SPDIF);
	pinmux_set_input(SWINPUT_SDL_SPDIF_SPDIF_IN1, 0x3); // Solo change
}

void mx6q_init_esai(void)
{
	/* ESAI_SCKT */
	pinmux_set_swmux(SWMUX_ENET_CRS_DV, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_ENET_CRS_DV, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_SCKT, 0x0);

	/* ESAI_SCKR */
	pinmux_set_swmux(SWMUX_ENET_MDIO, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_ENET_MDIO, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_SCKR, 0x0);

	/* ESAI_FST */
	pinmux_set_swmux(SWMUX_ENET_RXD1, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_ENET_RXD1, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_FST, 0x0);

	/* ESAI_TX3_RX2 */
	pinmux_set_swmux(SWMUX_ENET_TX_EN, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_ENET_TX_EN, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_SDO3_SDI2, 0x0);

	/* ESAI_TX2_RX3 */
	pinmux_set_swmux(SWMUX_GPIO_5, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_GPIO_5, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_SDO2_SDI3, 1);

	/* ESAI_TX4_RX1 */
	pinmux_set_swmux(SWMUX_ENET_TXD0, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_ENET_TXD0, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_SDO4_SDI1, 0);

	/* ESAI_TX5_RX0 */
	pinmux_set_swmux(SWMUX_ENET_MDC, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_ENET_MDC, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_SDO5_SDI0, 0);

	/* ESAI_TX0 */
	pinmux_set_swmux(SWMUX_GPIO_17, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_GPIO_17, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_SDO0, 0);

	/* ESAI_TX1 */
	pinmux_set_swmux(SWMUX_NANDF_CS3, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_NANDF_CS3, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_SDO1, 1);

	/* ESAI_FSR */
	pinmux_set_swmux(SWMUX_GPIO_9, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_GPIO_9, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_ESAI1_IPP_IND_FSR, 1);

	/* ESAI_INT */
	pinmux_set_swmux(SWMUX_SD2_CLK, MUX_CTL_MUX_MODE_ALT5); //GPIO_1_10
}

void mx6sdl_init_esai(void)
{
	/* ESAI_SCKT */
	pinmux_set_swmux(SWMUX_SDL_ENET_CRS_DV, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_ENET_CRS_DV, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_SCKT, 0x0);

	/* ESAI_SCKR */
	pinmux_set_swmux(SWMUX_SDL_ENET_MDIO, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_ENET_MDIO, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_SCKR, 0x0);

	/* ESAI_FST */
	pinmux_set_swmux(SWMUX_SDL_ENET_RXD1, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_ENET_RXD1, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_FST, 0x0);

	/* ESAI_TX3_RX2 */
	pinmux_set_swmux(SWMUX_SDL_ENET_TX_EN, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_ENET_TX_EN, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_SDO3_SDI2, 0x0);

	/* ESAI_TX2_RX3 */
	pinmux_set_swmux(SWMUX_SDL_GPIO_5, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_GPIO_5, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_SDO2_SDI3, 1);

	/* ESAI_TX4_RX1 */
	pinmux_set_swmux(SWMUX_SDL_ENET_TXD0, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_ENET_TXD0, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_SDO4_SDI1, 0);

	/* ESAI_TX5_RX0 */
	pinmux_set_swmux(SWMUX_SDL_ENET_MDC, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_ENET_MDC, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_SDO5_SDI0, 0);

	/* ESAI_TX0 */
	pinmux_set_swmux(SWMUX_SDL_GPIO_17, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_GPIO_17, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_SDO0, 0);

	/* ESAI_TX1 */
	pinmux_set_swmux(SWMUX_SDL_NANDF_CS3, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_NANDF_CS3, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_SDO1, 1);

	/* ESAI_FSR */
	pinmux_set_swmux(SWMUX_SDL_GPIO_9, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_padcfg(SWPAD_SDL_GPIO_9, MX6X_PAD_SETTINGS_ESAI);
	pinmux_set_input(SWINPUT_SDL_ESAI1_IPP_IND_FSR, 1);

	/* ESAI_INT */
	pinmux_set_swmux(SWMUX_SDL_SD2_CLK, MUX_CTL_MUX_MODE_ALT5); //GPIO_1_10
}

void mx6q_init_uart_2(void)
{
	/* TXD output */
	pinmux_set_swmux(SWMUX_GPIO_7, MUX_CTL_MUX_MODE_ALT4);
	pinmux_set_padcfg(SWPAD_GPIO_7, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_UART2_IPP_UART_RXD_MUX, 0x2);

	/* RXD input */
	pinmux_set_swmux(SWMUX_GPIO_8, MUX_CTL_MUX_MODE_ALT4);
	pinmux_set_padcfg(SWPAD_GPIO_8, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_UART2_IPP_UART_RXD_MUX, 0x3);

	/* CTS output */
	pinmux_set_swmux(SWMUX_SD4_DAT6, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SD4_DAT6, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_UART2_IPP_UART_RTS_B, 0x5);

	/* RTS input */
	pinmux_set_swmux(SWMUX_SD4_DAT5, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SD4_DAT5, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_UART2_IPP_UART_RTS_B, 0x4);
}

void mx6sdl_init_uart_1(void)
{
	/* TXD output */
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT10, MUX_CTL_MUX_MODE_ALT3);
	pinmux_set_padcfg(SWPAD_SDL_CSI0_DAT10, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_SDL_UART1_IPP_UART_RXD_MUX, 0);

	/* RXD input */
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT11, MUX_CTL_MUX_MODE_ALT3);
	pinmux_set_padcfg(SWPAD_SDL_CSI0_DAT11, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_SDL_UART1_IPP_UART_RXD_MUX, 1);
}


void mx6sdl_init_uart_2(void)
{
	/* TXD output */
	pinmux_set_swmux(SWMUX_SDL_GPIO_7, MUX_CTL_MUX_MODE_ALT4);
	pinmux_set_padcfg(SWPAD_SDL_GPIO_7, MX6X_PAD_SETTINGS_UART);

	/* RXD input */
	pinmux_set_swmux(SWMUX_SDL_GPIO_8, MUX_CTL_MUX_MODE_ALT4);
	pinmux_set_padcfg(SWPAD_SDL_GPIO_8, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_SDL_UART2_IPP_UART_RXD_MUX, 0x3);

	/* CTS output */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT6, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT6, MX6X_PAD_SETTINGS_UART);

	/* RTS Input */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT5, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_SD4_DAT5, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_SDL_UART2_IPP_UART_RTS_B, 0x4);
}

void mx6q_init_uart_3(void)
{
	/* RXD input */
	pinmux_set_swmux(SWMUX_SD4_CLK, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SD4_CLK, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_UART3_IPP_UART_RXD_MUX, 0x3);

	/* TXD output - note: input daisy chain must not be set */
	pinmux_set_swmux(SWMUX_SD4_CMD, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SD4_CMD, MX6X_PAD_SETTINGS_UART);

	/* RTS input */
	pinmux_set_swmux(SWMUX_EIM_EB3, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_EIM_EB3, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_UART3_IPP_UART_RTS_B, 0x1);

	/* CTS output - note: input daisy chain must not be set */
	pinmux_set_swmux(SWMUX_EIM_D30, MUX_CTL_MUX_MODE_ALT4);
	pinmux_set_padcfg(SWPAD_EIM_D30, MX6X_PAD_SETTINGS_UART);
}

void mx6sdl_init_uart_3(void)
{
	/* RXD input */
	pinmux_set_swmux(SWMUX_SDL_SD4_CLK, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_SD4_CLK, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_SDL_UART3_IPP_UART_RXD_MUX, 0x2);

	/* TXD output */
	pinmux_set_swmux(SWMUX_SDL_SD4_CMD, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_SD4_CMD, MX6X_PAD_SETTINGS_UART);

	/* RTS input */
	pinmux_set_swmux(SWMUX_SDL_EIM_EB3, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_padcfg(SWPAD_SDL_EIM_EB3, MX6X_PAD_SETTINGS_UART);
	pinmux_set_input(SWINPUT_SDL_UART3_IPP_UART_RTS_B, 0x3);

	/* CTS output */
	pinmux_set_swmux(SWMUX_SDL_EIM_D30, MUX_CTL_MUX_MODE_ALT4);
	pinmux_set_padcfg(SWPAD_SDL_EIM_D30, MX6X_PAD_SETTINGS_UART);
}

void mx6q_init_lvds0(void)
{
	/* Enable PWM3 */
	pinmux_set_swmux(SWMUX_SD4_DAT1, MUX_CTL_MUX_MODE_ALT5);

	out32(MX6X_GPIO2_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_GDIR) | (0x1 << 9));
	out32(MX6X_GPIO2_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_DR) | (0x1 << 9));

	/*
	 * Set TOUCH_INT_B mux mode to GPIO2[28] which allows an external LVDS display to send touch
	 * interrupts to the i.MX6
	 */
	pinmux_set_swmux(SWMUX_EIM_EB0, MUX_CTL_MUX_MODE_ALT5);
}

void mx6sdl_init_lvds0(void)
{
	/* Enable PARADISP_PWM3: set GPIO2[9] */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT1, MUX_CTL_MUX_MODE_ALT5);

	out32(MX6X_GPIO2_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_GDIR) | (0x1 << 9));
	out32(MX6X_GPIO2_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_DR) | (0x1 << 9));

	/* Set TOUCH_INT_B mux mode to GPIO2[28] which allows an external LVDS display to send touch
	 * interrupts to the i.MX6
	 */
	pinmux_set_swmux(SWMUX_SDL_EIM_EB0, MUX_CTL_MUX_MODE_ALT5);
}

void mx6q_init_lvds1(void)
{
	/* LVDS1 on Base board through edge finger connector */
	/* Enable LVDS1_PWM4 as GPIO2[10] */
	pinmux_set_swmux(SWMUX_SD4_DAT2, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO2_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_GDIR) | (0x1 << 10));
	out32(MX6X_GPIO2_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_DR) | (0x1 << 10));
}

void mx6sdl_init_lvds1(void)
{
	/* LVDS1 on Base board through edge finger connector */
	/* Enable LVDS1_PWM4 as GPIO2[10] */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT2, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO2_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_GDIR) | (0x1 << 10));
	out32(MX6X_GPIO2_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_DR) | (0x1 << 10));
}

void mx6q_init_hdmi(void)
{
	/* HDMI CEC I/O line */
	pinmux_set_swmux(SWMUX_EIM_A25, MUX_CTL_MUX_MODE_ALT6);
	pinmux_set_padcfg(SWPAD_EIM_A25, MX6X_PAD_SETTINGS_HDMI);
	pinmux_set_input(SWINPUT_HDMI_TX_ICECIN, 0x0);
}

void mx6sdl_init_hdmi(void)
{
	/* HDMI CEC I/O line */
	pinmux_set_swmux(SWMUX_SDL_EIM_A25, MUX_CTL_MUX_MODE_ALT6);
	pinmux_set_padcfg(SWPAD_SDL_EIM_A25, MX6X_PAD_SETTINGS_HDMI);
	pinmux_set_input(SWINPUT_SDL_HDMI_TX_ICECIN, 0x0);
}

void mx6q_init_lcd_panel(void)
{
	/* Used to configure backlight brightness  */
	pinmux_set_swmux(SWMUX_DI0_PIN4, MUX_CTL_MUX_MODE_ALT0);

	/* IPU1 Display Interface 0 clock */
	pinmux_set_swmux(SWMUX_DI0_DISP_CLK, MUX_CTL_MUX_MODE_ALT0);

	/* LCD EN */
	pinmux_set_swmux(SWMUX_DI0_PIN15, MUX_CTL_MUX_MODE_ALT0);

	/* LCD HSYNC */
	pinmux_set_swmux(SWMUX_DI0_PIN2, MUX_CTL_MUX_MODE_ALT0);

	/* LCD VSYNC */
	pinmux_set_swmux(SWMUX_DI0_PIN3, MUX_CTL_MUX_MODE_ALT0);

	/* Data Lines */
	pinmux_set_swmux(SWMUX_DISP0_DAT0, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT1, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT2, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT3, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT4, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT5, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT6, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT7, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT8, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT9, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT10, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT11, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT12, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT13, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT14, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT15, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT16, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT17, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT18, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT19, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT20, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_DISP0_DAT21, MUX_CTL_MUX_MODE_ALT0);
}

void mx6sdl_init_lcd_panel(void)
{
	/* IPU1: Used to configure backlight brightness  */
	pinmux_set_swmux(SWMUX_SDL_DI0_PIN4, MUX_CTL_MUX_MODE_ALT0);

	/* IPU1 Display Interface 0 clock */
	pinmux_set_swmux(SWMUX_SDL_DI0_DISP_CLK, MUX_CTL_MUX_MODE_ALT0);

	/* IPU1 LCD EN */
	pinmux_set_swmux(SWMUX_SDL_DI0_PIN15, MUX_CTL_MUX_MODE_ALT0);

	/* IPU1 LCD HSYNC */
	pinmux_set_swmux(SWMUX_SDL_DI0_PIN2, MUX_CTL_MUX_MODE_ALT0);

	/* IPU1 LCD VSYNC */
	pinmux_set_swmux(SWMUX_SDL_DI0_PIN3, MUX_CTL_MUX_MODE_ALT0);

	/* Data Lines */
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT0, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT1, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT2, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT3, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT4, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT5, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT6, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT7, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT8, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT9, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT10, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT11, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT12, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT13, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT14, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT15, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT16, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT17, MUX_CTL_MUX_MODE_ALT0);

	/* AUD5_TXFS */
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT18, MUX_CTL_MUX_MODE_ALT0);
	/* AUD5_RXD */
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT19, MUX_CTL_MUX_MODE_ALT0);
	/* VOL */
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT20, MUX_CTL_MUX_MODE_ALT0);
	/* D9 Blue user LED control */
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT21, MUX_CTL_MUX_MODE_ALT0);
}

void mx6q_init_can(void)
{
	/* configure Pin Mux for CAN interface */
	/* CAN1 TXCAN */
	pinmux_set_swmux(SWMUX_KEY_COL2, MUX_CTL_MUX_MODE_ALT2); //this pin is being shared between ENET and CAN//

	/* CAN1 RXCAN */
	pinmux_set_swmux(SWMUX_KEY_ROW2, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_input(SWINPUT_CAN1_IPP_IND_CANRX, 0);

	/* CAN2 TXCAN */
	pinmux_set_swmux(SWMUX_KEY_COL4, MUX_CTL_MUX_MODE_ALT0);

	/* CAN2 RXCAN */
	pinmux_set_swmux(SWMUX_KEY_ROW4, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_input(SWINPUT_CAN2_IPP_IND_CANRX, 0);

	/* CAN1 NERR- GPIO1_4  */
	pinmux_set_swmux(SWMUX_GPIO_4, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) & ~(0x1 << 4));

	/* CAN2 NERR: GPIO2[11]) */
	pinmux_set_swmux(SWMUX_SD4_DAT3, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO2_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_DR) & ~(0x1 << 11));
}

void mx6sdl_init_can(void)
{
	/* configure Pin Mux for CAN interface */
	/* CAN1 TXCAN */
	pinmux_set_swmux(SWMUX_SDL_KEY_COL2, MUX_CTL_MUX_MODE_ALT2); //this pin is being shared between ENET and CAN//

	/* CAN1 RXCAN */
	pinmux_set_swmux(SWMUX_SDL_KEY_ROW2, MUX_CTL_MUX_MODE_ALT2);
	pinmux_set_input(SWINPUT_SDL_CAN1_IPP_IND_CANRX, 1); // Solo change

	/* CAN2 TXCAN */
	pinmux_set_swmux(SWMUX_SDL_KEY_COL4, MUX_CTL_MUX_MODE_ALT0);

	/* CAN2 RXCAN */
	pinmux_set_swmux(SWMUX_SDL_KEY_ROW4, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_input(SWINPUT_SDL_CAN2_IPP_IND_CANRX, 0);

	/* CAN1 NERR: GPIO1[4]  */
	pinmux_set_swmux(SWMUX_SDL_GPIO_4, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO1_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO1_BASE + MX6X_GPIO_DR) & ~(0x1 << 4));

	/* CAN2 NERR: GPIO2[11] */
	pinmux_set_swmux(SWMUX_SDL_SD4_DAT3, MUX_CTL_MUX_MODE_ALT5);
	out32(MX6X_GPIO2_BASE + MX6X_GPIO_DR, in32(MX6X_GPIO2_BASE + MX6X_GPIO_DR) & ~(0x1 << 11));
}

void mx6q_init_pmic(void)
{
	/* Set up GPIO5[16] as input, enable FALLING interrupt */
	pinmux_set_swmux(SWMUX_DISP0_DAT22, MUX_CTL_MUX_MODE_ALT5);

	out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) & (~(1<<16)));
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_ICR2, in32(MX6X_GPIO5_BASE + MX6X_GPIO_ICR2) | (MX6_INTR_CFG_FALLING << GPIO5_16_ICR2_SHIFT));
}

void mx6sdl_init_pmic(void)
{
	/* Set up GPIO5[16] as input, enable FALLING interrupt */
	pinmux_set_swmux(SWMUX_SDL_DISP0_DAT22, MUX_CTL_MUX_MODE_ALT5);

	out32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR, in32(MX6X_GPIO5_BASE + MX6X_GPIO_GDIR) & (~(1<<16)));
	out32(MX6X_GPIO5_BASE + MX6X_GPIO_ICR2, in32(MX6X_GPIO5_BASE + MX6X_GPIO_ICR2) | (MX6_INTR_CFG_FALLING << GPIO5_16_ICR2_SHIFT));
}

void mx6q_init_video(void)
{
	pinmux_set_swmux(SWMUX_CSI0_PIXCLK, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_MCLK, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_VSYNC, MUX_CTL_MUX_MODE_ALT0);

	/* Data Lines */
	pinmux_set_swmux(SWMUX_CSI0_DAT4, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT5, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT6, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT7, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT8, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT9, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT10, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT11, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT12, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT13, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT14, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT15, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT16, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT17, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT18, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_CSI0_DAT19, MUX_CTL_MUX_MODE_ALT0);

	/* Route ADV7180 parallel video input to IPU's Camera Sensor Input (CSI) 0*/
	uint32_t gpr1;
	gpr1 = in32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1);
	gpr1 &= ~MX6DQ_GPR1_MIPI_IPU1_MUX_MASK;
	gpr1 |= MX6DQ_GPR1_MIPI_IPU1_PARALLEL;
	out32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR1, gpr1);
}

void mx6sdl_init_video(void)
{
	pinmux_set_swmux(SWMUX_SDL_CSI0_PIXCLK, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_MCLK, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_VSYNC, MUX_CTL_MUX_MODE_ALT0);

	/* Data Lines */
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT4, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT5, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT6, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT7, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT8, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT9, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT10, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT11, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT12, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT13, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT14, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT15, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT16, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT17, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT18, MUX_CTL_MUX_MODE_ALT0);
	pinmux_set_swmux(SWMUX_SDL_CSI0_DAT19, MUX_CTL_MUX_MODE_ALT0);

	uint32_t gpr13;
	gpr13 = in32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR13);

	/* Route ADV7180 parallel video input to IPU's Camera Sensor Input (CSI) 0*/
	gpr13 &= ~MX6SDL_GPR13_IPU_CSI0_MUX_MASK;
	gpr13 |= MX6SDL_GPR13_IPU_CSI0_IPU_CSI0;

	/*
	 * Route MIPI/CSI2 video input 1 to IPU's Camera Sensor Input (CSI) 1 
	 * Sabre-ARD schematics indicate "ADI Monza SD video decoder card" is needed
	 * to verify MIPI/CSI-2 video input.
	 */
	gpr13 &= ~MX6SDL_GPR13_IPU_CSI1_MUX_MASK;
	gpr13 |= MX6SDL_GPR13_IPU_CSI1_MIPI_CSI1;
	out32(MX6X_IOMUXC_BASE + MX6X_IOMUX_GPR13, gpr13);

}

void mx6x_init_displays(void)
{
	if ( get_mx6_chip_type() == MX6_CHIP_TYPE_QUAD_OR_DUAL)
	{
		mx6q_init_lvds0();
		mx6q_init_lvds1();
		mx6q_init_hdmi();
		mx6q_init_lcd_panel();
		mx6q_init_video();
	}
	else if (get_mx6_chip_type() == MX6_CHIP_TYPE_DUAL_LITE_OR_SOLO)
	{
		mx6sdl_init_lvds0();
		mx6sdl_init_lvds1();
		mx6sdl_init_hdmi();
		mx6sdl_init_lcd_panel();
		mx6sdl_init_video();
	}
}

#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/boards/imx6x/sabreARD/mx6x_gpio.c $ $Rev: 780399 $")
#endif
