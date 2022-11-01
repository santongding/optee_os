// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (c) 2016-2020, Linaro Limited
 * Copyright (c) 2014, STMicroelectronics International N.V.
 */

#include <arm.h>
#include <console.h>
#include <drivers/gic.h>
#include <drivers/tpm2_mmio.h>
#include <drivers/tpm2_ptp_fifo.h>
#include <drivers/tzc400.h>
#include <drivers/serial.h>
#include <initcall.h>
#include <keep.h>
#include <kernel/boot.h>
#include <kernel/interrupt.h>
#include <kernel/misc.h>
#include <kernel/notif.h>
#include <kernel/panic.h>
#include <kernel/spinlock.h>
#include <kernel/tee_time.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <platform_config.h>
#include <sm/psci.h>
#include <stdint.h>
#include <string.h>
#include <trace.h>

static struct gic_data gic_data __nex_bss;
static struct serial_chip hf_serial_chip __nex_bss;

#if defined(CFG_DRIVERS_TPM2_MMIO)
register_phys_mem_pgdir(MEM_AREA_IO_SEC, TPM2_BASE, TPM2_REG_SIZE);
#endif
#if defined(PLATFORM_FLAVOR_fvp)
register_phys_mem(MEM_AREA_RAM_SEC, TZCDRAM_BASE, TZCDRAM_SIZE);
#endif
#if defined(PLATFORM_FLAVOR_qemu_virt)
register_phys_mem_pgdir(MEM_AREA_IO_SEC, SECRAM_BASE, SECRAM_COHERENT_SIZE);
#endif
#ifdef DRAM0_BASE
register_ddr(DRAM0_BASE, DRAM0_SIZE);
#endif
#ifdef DRAM1_BASE
register_ddr(DRAM1_BASE, DRAM1_SIZE);
#endif

#ifdef GIC_BASE

register_phys_mem_pgdir(MEM_AREA_IO_SEC, GICD_BASE, GIC_DIST_REG_SIZE);
register_phys_mem_pgdir(MEM_AREA_IO_SEC, GICC_BASE, GIC_DIST_REG_SIZE);

void main_init_gic(void)
{
#if defined(CFG_WITH_ARM_TRUSTED_FW)
	/* On ARMv8, GIC configuration is initialized in ARM-TF */
	gic_init_base_addr(&gic_data, GIC_BASE + GICC_OFFSET,
			   GIC_BASE + GICD_OFFSET);
#else
	gic_init(&gic_data, GIC_BASE + GICC_OFFSET, GIC_BASE + GICD_OFFSET);
#endif
	itr_init(&gic_data.chip);
}

#if !defined(CFG_WITH_ARM_TRUSTED_FW)
void main_secondary_init_gic(void)
{
	gic_cpu_init(&gic_data);
}
#endif

#endif

void itr_core_handler(void)
{
	gic_it_handle(&gic_data);
}
extern void init_serial_chip(struct serial_chip * chip);

void console_init(void)
{
	init_serial_chip(&hf_serial_chip);
	register_serial_console(&hf_serial_chip);
}

#if defined(CFG_DRIVERS_TPM2_MMIO)
static TEE_Result init_tpm2(void)
{
	enum tpm2_result res = TPM2_OK;

	res = tpm2_mmio_init(TPM2_BASE);
	if (res) {
		EMSG("Failed to initialize TPM2 MMIO");
		return TEE_ERROR_GENERIC;
	}

	DMSG("TPM2 Chip initialized");

	return TEE_SUCCESS;
}
driver_init(init_tpm2);
#endif /* defined(CFG_DRIVERS_TPM2_MMIO) */

#ifdef CFG_TZC400
register_phys_mem_pgdir(MEM_AREA_IO_SEC, TZC400_BASE, TZC400_REG_SIZE);

static TEE_Result init_tzc400(void)
{
	void *va;

	DMSG("Initializing TZC400");

	va = phys_to_virt(TZC400_BASE, MEM_AREA_IO_SEC, TZC400_REG_SIZE);
	if (!va) {
		EMSG("TZC400 not mapped");
		panic();
	}

	tzc_init((vaddr_t)va);
	tzc_dump_state();

	return TEE_SUCCESS;
}

service_init(init_tzc400);
#endif /*CFG_TZC400*/

#if defined(PLATFORM_FLAVOR_qemu_virt)
static void release_secondary_early_hpen(size_t pos)
{
	struct mailbox {
		uint64_t ep;
		uint64_t hpen[];
	} *mailbox;

	if (cpu_mmu_enabled())
		mailbox = phys_to_virt(SECRAM_BASE, MEM_AREA_IO_SEC,
				       SECRAM_COHERENT_SIZE);
	else
		mailbox = (void *)SECRAM_BASE;

	if (!mailbox)
		panic();

	mailbox->ep = TEE_LOAD_ADDR;
	dsb_ishst();
	mailbox->hpen[pos] = 1;
	dsb_ishst();
	sev();
}

int psci_cpu_on(uint32_t core_id, uint32_t entry, uint32_t context_id)
{
	size_t pos = get_core_pos_mpidr(core_id);
	static bool core_is_released[CFG_TEE_CORE_NB_CORE];

	if (!pos || pos >= CFG_TEE_CORE_NB_CORE)
		return PSCI_RET_INVALID_PARAMETERS;

	DMSG("core pos: %zu: ns_entry %#" PRIx32, pos, entry);

	if (core_is_released[pos]) {
		EMSG("core %zu already released", pos);
		return PSCI_RET_DENIED;
	}
	core_is_released[pos] = true;

	boot_set_core_ns_entry(pos, entry, context_id);
	release_secondary_early_hpen(pos);

	return PSCI_RET_SUCCESS;
}
#endif /*PLATFORM_FLAVOR_qemu_virt*/
