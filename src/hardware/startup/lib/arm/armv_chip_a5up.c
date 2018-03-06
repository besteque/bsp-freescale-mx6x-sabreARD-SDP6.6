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



#include "startup.h"

/*
 * Configuration for non-MPCore Cortex-A5 implementations:
 * - cache/page callouts use normal ARMv7 operations
 * - pte encodings use normal ARMv7 encodings
 *
 * NOTE: we need to set bit 10 (ARM_MMU_CR_F) to enable SWP instructions.
 *       These were deprecated in ARMv6 and Cortex-A5 disables them by
 *       default causing them to generate illegal instruction exceptions.
 */
const struct armv_chip armv_chip_a5up = {
	.cpuid		= 0xc050,
	.name		= "Cortex A5",
	.mmu_cr_set	= ARM_MMU_CR_XP | ARM_MMU_CR_I | ARM_MMU_CR_Z | ARM_MMU_CR_F,
	.mmu_cr_clr	= 0,
	.cycles		= 2,
	.cache		= &armv_cache_a5up,
	.power		= &power_v7_wfi,
	.flush		= &page_flush_a5up,
	.deferred	= &page_flush_deferred_a5up,
	.pte		= &armv_pte_v7wa,
	.pte_wa		= &armv_pte_v7wa,
	.pte_wb		= &armv_pte_v7wb,
	.pte_wt		= &armv_pte_v7wt,
	.setup		= armv_setup_a9up,
	.ttb_attr	= ARM_TTBR_RGN_WA | ARM_TTBR_IRGN_WA,
	.pte_attr	= ARM_PTE_V6_SP_XN | ARM_PTE_V6_WA,
};



#if defined(__QNXNTO__) && defined(__USESRCVERSION)
#include <sys/srcversion.h>
__SRCVERSION("$URL: http://svn.ott.qnx.com/product/branches/6.6.0/trunk/hardware/startup/lib/arm/armv_chip_a5up.c $ $Rev: 777256 $")
#endif
