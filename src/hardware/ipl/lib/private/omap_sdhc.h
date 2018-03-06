/*
 * $QNXLicenseC:
 * Copyright 2013, QNX Software Systems.
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

#ifndef _OMAP_SDHC_INCLUDED
#define _OMAP_SDHC_INCLUDED

#include "sdmmc.h"
#include "omap_adma.h"
#include "omap3_edma.h"
#include "omap_sdma.h"

#define MMCHS_FIFO_SIZE				512

#define OMAP_DID_34XX				0x3430
#define OMAP_DID_44XX				0x4430
#define OMAP_DID_54XX				0x5430
#define OMAP_DID_54XX_ES2			0x5432
#define OMAP_DID_DRA7XX				0x5752

typedef enum {
	OMAP_SDMMC_DMA_NONE = 0,
	OMAP_SDMMC_ADMA,
	OMAP_SDMMC_EDMA,
	OMAP_SDMMC_SDMA
} omap_sdmmc_dma_t;

#define MMCHS_PWR_SWITCH_TIMEOUT    5000		// 5ms

// Register Descriptions
#define OMAP_MMCHS_HL_REV			0x000		// (OMAP 4 only)
#define OMAP_MMCHS_HL_HWINFO		0x004		// (OMAP 4 only)
#define OMAP_MMCHS_HL_SYSCONFIG		0x010		// (OMAP 4 only)

#define OMAP_MMCHS_SYSCONFIG		0x110		// System Configuration Register
	#define SYSCONFIG_AUTOIDLE		(1 << 0)	// Clock auto idle
	#define SYSCONFIG_SOFTRESET		(1 << 1)	// Software reset

#define OMAP_MMCHS_SYSSTATUS		0x114		// System Status Register
	#define SYSSTATUS_RESETDONE		(1 << 0)	// Reset done

#define OMAP_MMCHS_CSRE				0x124		// Card status response error
#define OMAP_MMCHS_SYSTEST			0x128		// System Test Register
#define OMAP_MMCHS_CON				0x12C		// Configuration register
	#define CON_OD					(1 << 0)	// Card Open Drain Mode
	#define CON_INIT				(1 << 1)	// Send initialization stream
	#define CON_DW8					(1 << 5)	// MMC 8-bit width
	#define CON_DEBOUNCE_8MS		(3 <<  9)
	#define CON_PADEN				(1 << 15)	// Control Power for MMC Lines
	#define CON_CLKEXTFREE			(1 << 16)	// External clock free running
	#define CON_DDR					(1 << 19)	// DDR mode
	#define CON_MNS					(1 << 20)	// DMA master

#define OMAP_MMCHS_PWCNT			0x130		// Power counter register
#define OMAP_MMCHS_DLL				0x134		// DLL control and status register
	#define DLL_SLAVE_RATIO_SHIFT   6           // Position of SLAVE_RATIO in MMCHS_DLL register
	#define DLL_FORCE_SRC_SHIFT     13          // Position of FORCE_SR_C in MMCHS_DLL register
	#define DLL_FORCE_SRC_MAX       0x7f
	#define DLL_FORCE_SRC_MSK       (0x7f << 13)
	#define DLL_LOCK                (1 << 0)    // Master DLL lock status
	#define DLL_CALIB               (1 << 1)    // Enable slave DLL to update delay values
	#define DLL_UNLOCK_STICKY       (1 << 2)    // Asserted when single period  exceeds
	#define DLL_UNLOCK_CLEAR        (1 << 3)    // Clear unlock sticky flag
	#define DLL_FORCE_VALUE         (1 << 12)   // Put forced values to slave DLL
	#define DLL_SLAVE_RATIO_MSK     (0x3f << 6) // Slave delay ratio mask


#define OMAP_MMCHS_BLK				0x204		// Transfer Length Configuration register
#define OMAP_MMCHS_ARG				0x208		// Command argument Register
#define OMAP_MMCHS_CMD				0x20C		// Command and transfer mode register
	#define CMD_DE					(1 << 0)	// DMA Enable
	#define CMD_BCE					(1 << 1)	// Block Count Enable
	#define CMD_ACEN				(1 << 2)	// Auto CMD12 Enable
	#define CMD_DDIR				(1 << 4)	// Data Transfer Direction Read
	#define CMD_MBS					(1 << 5)	// Multi Block Select
	#define CMD_RSP_TYPE_136		(1 << 16)	// Response length 136 bit
	#define CMD_RSP_TYPE_48			(2 << 16)	// Response length 48 bit
	#define CMD_RSP_TYPE_48b		(3 << 16)	// Response length 48 bit with busy after response
	#define CMD_RSP_TYPE_MASK		(3 << 16)
	#define CMD_CCCE				(1 << 19)	// Comamnd CRC Check Enable
	#define CMD_CICE				(1 << 20)	// Comamnd Index Check Enable
	#define CMD_DP					(1 << 21)	// Data Present
	#define CMD_TYPE_CMD12			(3 << 22)	// CMD12 or CMD52 "I/O Abort"

#define OMAP_MMCHS_RSP10			0x210		// Command response[31:0] Register
#define OMAP_MMCHS_RSP32			0x214		// Command response[63:32] Register
#define OMAP_MMCHS_RSP54			0x218		// Command response[95:64] Register
#define OMAP_MMCHS_RSP76			0x21C		// Command response[127:96] Register
#define OMAP_MMCHS_DATA				0x220		// Data Register
#define OMAP_MMCHS_PSTATE			0x224		// Present state register
	#define PSTATE_CMDI				(1 << 0)	// Command inhibit (mmci_cmd).
	#define PSTATE_DATI				(1 << 1)	// Command inhibit (mmci_dat).
	#define PSTATE_DLA				(1 << 2)	// mmci_dat line active
	#define PSTATE_WTA				(1 << 8)	// Write Transfer Active
	#define PSTATE_RTA				(1 << 9)	// Read Transfer Active
	#define PSTATE_BWE				(1 << 10)	// Buffer Write Ready
	#define PSTATE_BRE				(1 << 11)	// Buffer Read Ready
	#define PSTATE_DLEV_MSK			(0xf << 20)
	#define PSTATE_CLEV				(1 << 24)

#define OMAP_MMCHS_HCTL				0x228		// Host Control register
	#define HCTL_DTW4				(1 << 1)	// Data transfer width 4 bit
	#define HCTL_HSE				(1 << 2)    // High Speed (omap 4)
	#define HCTL_DMAS_ADMA2			(2 << 3)	// DMA Select ADMA2
	#define HCTL_SDBP				(1 << 8)	// SD bus power
	#define HCTL_SDVS1V8			(5 << 9)	// SD bus voltage 1.8V
	#define HCTL_SDVS3V0			(6 << 9)	// SD bus voltage 3.0V
	#define HCTL_SDVS3V3			(7 << 9)	// SD bus voltage 3.3V
	#define	HCTL_SDVS_MSK			(7 << 9)

#define OMAP_MMCHS_SYSCTL			0x22C		// SD system control register
	#define SYSCTL_ICE				(1 << 0)	// Internal Clock Enable
	#define SYSCTL_ICS				(1 << 1)	// Internal Clock Stable
	#define SYSCTL_CEN				(1 << 2)	// Clock Enable
	#define SYSCTL_DTO_MASK			(0xF << 16) // Data timeout counter value and busy timeout
	#define SYSCTL_DTO_MAX			(0xE << 16) // Timeout = TCF x 2^27
	#define SYSCTL_SRA				(1 << 24)	// Software reset for all
	#define SYSCTL_SRC				(1 << 25)	// Software reset for mmci_cmd line
	#define SYSCTL_SRD				(1 << 26)	// Software reset for mmci_dat line

#define OMAP_MMCHS_STAT				0x230		// Interrupt status register
#define OMAP_MMCHS_IE				0x234		// Interrupt SD enable register
#define OMAP_MMCHS_ISE				0x238		// Interrupt signal enable register
	#define INTR_CC					(1 << 0)	// Command Complete
	#define INTR_TC					(1 << 1)	// Transfer Complete
	#define INTR_BGE				(1 << 2)	// Block Gap Event
	#define INTR_BWR				(1 << 4)	// Buffer Write Ready interrupt
	#define INTR_BRR				(1 << 5)	// Buffer Read Ready interrupt
	#define INTR_CIRQ				(1 << 8)	// Card interrupt
	#define INTR_OBI				(1 << 9)	// Out-Of-Band interrupt
	#define INTR_ERRI				(1 << 15)	// Error interrupt
	#define INTR_CTO				(1 << 16)	// Command Timeout error
	#define INTR_CCRC				(1 << 17)	// Command CRC error
	#define INTR_CEB				(1 << 18)	// Command End Bit error
	#define INTR_CIE				(1 << 19)	// Command Index error
	#define INTR_DTO				(1 << 20)	// Data Timeout error
	#define INTR_DCRC				(1 << 21)	// Data CRC error
	#define INTR_DEB				(1 << 22)	// Data End Bit error
	#define INTR_ACE				(1 << 24)	// ACMD12 error
	#define INTR_CERR				(1 << 28)	// Card error
	#define INTR_BADA				(1 << 29)	// Bad Access to Data Space
	#define INTR_ALL				0x33ff87ff	// old 0x117F8033
	#define STAT_CLR_MSK			0x117f8033

#define OMAP_MMCHS_AC12              0x23C		// Auto CMD12 Error Status Register
#define OMAP_MMCHS_HCTL2             0x23C
	#define HCTL2_PRESET_VAL        (1 << 31)
	#define HCTL2_ASYNC_INT         (1 << 30)
	#define HCTL2_TUNED_CLK         (1 << 23)
	#define HCTL2_EXEC_TUNING       (1 << 22)
	#define HCTL2_DRV_TYPE_MSK      (0x3 << 20)
	#define HCTL2_DRV_TYPE_D        (0x3 << 20)
	#define HCTL2_DRV_TYPE_C        (0x2 << 20)
	#define HCTL2_DRV_TYPE_A        (0x1 << 20)
	#define HCTL2_DRV_TYPE_B        (0x0 << 20)
	#define HCTL2_SIG_1_8V          (1 << 19)
	#define HCTL2_MODE( _c )        ( (_c) & HCTL2_MODE_MSK )
	#define HCTL2_MODE_MSK          (0x7 << 16)
	#define HCTL2_MODE_HS200        (0x5 << 16)
	#define HCTL2_MODE_DDR50        (0x4 << 16)
	#define HCTL2_MODE_SDR104       (0x3 << 16)
	#define HCTL2_MODE_SDR50        (0x2 << 16)
	#define HCTL2_MODE_SDR25        (0x1 << 16)

#define OMAP_MMCHS_CAPA				0x240		// Capabilities register
	#define CAPA_VS3V3				(1 << 24)	// 3.3V
	#define CAPA_VS3V0				(1 << 25)	// 3.0V
	#define CAPA_VS1V8				(1 << 26)	// 1.8V

#define OMAP_MMCHS_CAPA2             0x244   // bits 32 to 64
	#define CAP_CLK_MULT( _cap )    ( ( ( _cap ) & 0xff0000 ) >> 16 )
	#define CAP_TUNING_MODE( _cap ) ( ( ( _cap ) & 0xe000 ) >> 14 )
	#define CAP_TUNE_SDR50          (1 << 13)
	#define CAP_TIMER_COUNT( _cap ) ( ( ( _cap ) & 0xf00 ) >> 8 )
	#define CAP_TIMER_OTHER			0x0f    // get info from other source
	#define CAP_DRIVE_TYPE_D        (1 << 6)
	#define CAP_DRIVE_TYPE_C        (1 << 5)
	#define CAP_DRIVE_TYPE_A        (1 << 4)
	#define CAP_DDR50               (1 << 2)
	#define CAP_SDR104              (1 << 1)
	#define CAP_SDR50               (1 << 0)

#define OMAP_MMCHS_CUR_CAPA			0x248		// Maximum current capabilities Register
#define OMAP_MMCHS_FE				0x250		// Force Event (OMAP 4 only)
#define OMAP_MMCHS_ADMAES			0x254		// ADMA Error Status (OMAP 4 only)
#define OMAP_MMCHS_ADMASAL			0x258		// ADMA system address low bits
#define OMAP_MMCHS_REV				0x2FC		// Versions Register
	#define REV_SREV(_r)            ( ( (_r) >> 16 ) & 0xff)
	#define REV_SREV_V1             0x0
	#define REV_SREV_V2             0x1
	#define REV_SREV_V3             0x2



typedef struct omap_edma_ext
{
	unsigned	dma_pbase;			// DMA base address
	int			dma_chnl;			// DMA channel
} omap_edma_ext_t;

typedef struct omap_adma_ext
{
	omap_adma32_t	*adma_des;
} omap_adma_ext_t;

typedef struct omap_sdma_ext
{
	unsigned	dma_pbase;			// DMA base address
	int			dma_chnl;			// DMA channel
	int			dma_rreq;			// DMA request line for Rx
	int			dma_treq;			// DMA request line for Tx
} omap_sdma_ext_t;

extern void omap_sdmmc_init_hc(sdmmc_t *sdmmc, unsigned base, unsigned clock, int verbose, int dma, void *dma_ext);
extern void omap_pwr(sdmmc_t *sdmmc, int vdd);
extern int omap_signal_voltage( sdmmc_t *sdmmc, int signal_voltage );
extern int omap_tune( sdmmc_t *sdmmc, int op );
extern int omap_init_ctrl_v18(sdmmc_t *sdmmc);

#endif /* #ifndef _OMAP_SDHC_INCLUDED */


#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/ipl/lib/private/omap_sdhc.h $ $Rev: 778407 $")
#endif
