PLATFORM_FLAVOR ?= fvp
include core/arch/arm/cpu/cortex-armv8-0.mk
platform-debugger-arm := 1



ifeq ($(platform-debugger-arm),1)
# ARM debugger needs this
platform-cflags-debug-info = -gdwarf-2
platform-aflags-debug-info = -gdwarf-2
endif

ifeq ($(platform-flavor-armv8),1)
$(call force,CFG_WITH_ARM_TRUSTED_FW,y)
endif

$(call force,CFG_GIC,y)
$(call force,CFG_PL011,y)
$(call force,CFG_SECURE_TIME_SOURCE_CNTPCT,y)

ifeq ($(CFG_CORE_TPM_EVENT_LOG),y)
# NOTE: Below values for the TPM event log are implementation
# dependent and used mostly for debugging purposes.
# Care must be taken to properly configure them if used.
CFG_TPM_LOG_BASE_ADDR ?= 0x402c951
CFG_TPM_MAX_LOG_SIZE ?= 0x200
endif

$(call force,CFG_ARM64_core,y)
$(call force,CFG_CORE_SEL1_SPMC,y)

CFG_WITH_STATS ?= y
CFG_ENABLE_EMBEDDED_TESTS ?= y

CFG_TEE_CORE_NB_CORE = 8
ifeq ($(CFG_CORE_SEL2_SPMC),y)
CFG_TZDRAM_START ?= 0x06281000
CFG_TZDRAM_SIZE  ?= 0x01D80000
else
CFG_TZDRAM_START ?= 0x06382000
CFG_TZDRAM_SIZE  ?= 0x002fe000
endif
CFG_SHMEM_START  ?= 0x83000000
CFG_SHMEM_SIZE   ?= 0x00200000
# DRAM1 is defined above 4G
$(call force,CFG_CORE_LARGE_PHYS_ADDR,y)
$(call force,CFG_CORE_ARM64_PA_BITS,36)