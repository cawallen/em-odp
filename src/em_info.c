/*
 *   Copyright (c) 2015-2023, Nokia Solutions and Networks
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Copyright (c) 2018, Linaro Limited
 * Copyright (c) 2020-2021, Nokia
 * All rights reserved.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 */

/* sysinfo printouts from the ODP example: odp_sysinfo.c */
/* SW ISA detection from the <arch>/odp_sysinfo_parse.c files */

#include "em_include.h"

static const char *cpu_arch_name(odp_cpu_arch_t cpu_arch)
{
	switch (cpu_arch) {
	case ODP_CPU_ARCH_ARM:
		return "ARM";
	case ODP_CPU_ARCH_MIPS:
		return "MIPS";
	case ODP_CPU_ARCH_PPC:
		return "PPC";
	case ODP_CPU_ARCH_RISCV:
		return "RISC-V";
	case ODP_CPU_ARCH_X86:
		return "x86";
	default:
		return "Unknown";
	}
}

static const char *arm_isa_name(odp_cpu_arch_arm_t isa)
{
	switch (isa) {
	case ODP_CPU_ARCH_ARMV6:
		return "ARMv6";
	case ODP_CPU_ARCH_ARMV7:
		return "ARMv7-A";
	case ODP_CPU_ARCH_ARMV8_0:
		return "ARMv8.0-A";
	case ODP_CPU_ARCH_ARMV8_1:
		return "ARMv8.1-A";
	case ODP_CPU_ARCH_ARMV8_2:
		return "ARMv8.2-A";
	case ODP_CPU_ARCH_ARMV8_3:
		return "ARMv8.3-A";
	case ODP_CPU_ARCH_ARMV8_4:
		return "ARMv8.4-A";
	case ODP_CPU_ARCH_ARMV8_5:
		return "ARMv8.5-A";
	case ODP_CPU_ARCH_ARMV8_6:
		return "ARMv8.6-A";
	case ODP_CPU_ARCH_ARMV8_7:
		return "ARMv8.7-A";
	case ODP_CPU_ARCH_ARMV9_0:
		return "ARMv9.0-A";
	case ODP_CPU_ARCH_ARMV9_1:
		return "ARMv9.1-A";
	case ODP_CPU_ARCH_ARMV9_2:
		return "ARMv9.2-A";
#if ODP_VERSION_API_NUM(1, 42, 1) <= ODP_VERSION_API
	case ODP_CPU_ARCH_ARMV8_8:
		return "ARMv8.8-A";
	case ODP_CPU_ARCH_ARMV8_9:
		return "ARMv8.9-A";
	case ODP_CPU_ARCH_ARMV9_3:
		return "ARMv9.3-A";
#endif
	default:
		return "Unknown";
	}
}

static const char *x86_isa_name(odp_cpu_arch_x86_t isa)
{
	switch (isa) {
	case ODP_CPU_ARCH_X86_I686:
		return "x86_i686";
	case ODP_CPU_ARCH_X86_64:
		return "x86_64";
	default:
		return "Unknown";
	}
}

static const char *cpu_arch_isa_name(odp_cpu_arch_t cpu_arch,
				     odp_cpu_arch_isa_t cpu_arch_isa)
{
	switch (cpu_arch) {
	case ODP_CPU_ARCH_ARM:
		return arm_isa_name(cpu_arch_isa.arm);
	case ODP_CPU_ARCH_MIPS:
		return "Unknown";
	case ODP_CPU_ARCH_PPC:
		return "Unknown";
	case ODP_CPU_ARCH_RISCV:
		return "Unknown";
	case ODP_CPU_ARCH_X86:
		return x86_isa_name(cpu_arch_isa.x86);
	default:
		return "Unknown";
	}
}

/*
 * Detect the ARM ISA used when compiling the em-odp library.
 * (based on the <arch>/odp_sysinfo_parse.c files)
 */
static odp_cpu_arch_arm_t detect_sw_isa_arm(void)
{
#if defined(__ARM_ARCH)
	if (__ARM_ARCH == 8) {
	#ifdef __ARM_FEATURE_QRDMX
		/* v8.1 or higher */
		return ODP_CPU_ARCH_ARMV8_1;
	#else
		return ODP_CPU_ARCH_ARMV8_0;
	#endif
	}

	if (__ARM_ARCH == 9) {
		/* v9.0 or higher */
		return ODP_CPU_ARCH_ARMV9_0;
	}

	if (__ARM_ARCH >= 800) {
		/*
		 * ACLE 2018 defines that from v8.1 onwards the value includes
		 * the minor version number: __ARM_ARCH = X * 100 + Y
		 * E.g. for Armv8.1 __ARM_ARCH = 801
		 */
		int major = __ARM_ARCH / 100;
		int minor = __ARM_ARCH - (major * 100);

		if (major == 8) {
			switch (minor) {
			case 0:
				return ODP_CPU_ARCH_ARMV8_0;
			case 1:
				return ODP_CPU_ARCH_ARMV8_1;
			case 2:
				return ODP_CPU_ARCH_ARMV8_2;
			case 3:
				return ODP_CPU_ARCH_ARMV8_3;
			case 4:
				return ODP_CPU_ARCH_ARMV8_4;
			case 5:
				return ODP_CPU_ARCH_ARMV8_5;
			case 6:
				return ODP_CPU_ARCH_ARMV8_6;
			case 7:
				return ODP_CPU_ARCH_ARMV8_7;
#if ODP_VERSION_API_NUM(1, 42, 1) <= ODP_VERSION_API
			case 8:
				return ODP_CPU_ARCH_ARMV8_8;
			case 9:
				return ODP_CPU_ARCH_ARMV8_9;
#endif
			default:
				return ODP_CPU_ARCH_ARM_UNKNOWN;
			}
		} else if (major == 9) {
			switch (minor) {
			case 0:
				return ODP_CPU_ARCH_ARMV9_0;
			case 1:
				return ODP_CPU_ARCH_ARMV9_1;
			case 2:
				return ODP_CPU_ARCH_ARMV9_2;
#if ODP_VERSION_API_NUM(1, 42, 1) <= ODP_VERSION_API
			case 3:
				return ODP_CPU_ARCH_ARMV9_3;
#endif
			default:
				return ODP_CPU_ARCH_ARM_UNKNOWN;
			}
		}
	}
#endif
	return ODP_CPU_ARCH_ARM_UNKNOWN;
}

/*
 * Detect the x86 ISA used when compiling the em-odp library.
 * (based on the <arch>/odp_sysinfo_parse.c files)
 */
static odp_cpu_arch_x86_t detect_sw_isa_x86(void)
{
	odp_cpu_arch_x86_t isa_x86 = ODP_CPU_ARCH_X86_UNKNOWN;

#if defined __x86_64 || defined __x86_64__
	isa_x86 = ODP_CPU_ARCH_X86_64;
#elif defined __i686 || defined __i686__
	isa_x86 = ODP_CPU_ARCH_X86_I686;
#endif
	return isa_x86;
}

static odp_cpu_arch_mips_t detect_sw_isa_mips(void)
{
	return ODP_CPU_ARCH_MIPS_UNKNOWN;
}

static odp_cpu_arch_ppc_t detect_sw_isa_ppc(void)
{
	return ODP_CPU_ARCH_PPC_UNKNOWN;
}

static odp_cpu_arch_riscv_t detect_sw_isa_riscv(void)
{
	return ODP_CPU_ARCH_RISCV_UNKNOWN;
}

/*
 * Detect the SW CPU ISA used when compiling the em-odp library.
 * (based on the <arch>/odp_sysinfo_parse.c files)
 */
static odp_cpu_arch_isa_t detect_sw_isa(odp_cpu_arch_t cpu_arch)
{
	odp_cpu_arch_isa_t sw_isa;

	switch (cpu_arch) {
	case ODP_CPU_ARCH_ARM:
		sw_isa.arm = detect_sw_isa_arm();
		break;
	case ODP_CPU_ARCH_MIPS:
		sw_isa.mips = detect_sw_isa_mips();
		break;
	case ODP_CPU_ARCH_PPC:
		sw_isa.ppc = detect_sw_isa_ppc();
		break;
	case ODP_CPU_ARCH_RISCV:
		sw_isa.riscv = detect_sw_isa_riscv();
		break;
	case ODP_CPU_ARCH_X86:
		sw_isa.x86 = detect_sw_isa_x86();
		break;
	default:
		sw_isa.arm = ODP_CPU_ARCH_ARM_UNKNOWN;
		break;
	}

	return sw_isa;
}

void print_core_map_info(void)
{
	EM_PRINT("Core mapping: EM-core <-> phys-core <-> ODP-thread\n");

	for (int logic_core = 0; logic_core < em_core_count(); logic_core++) {
		EM_PRINT("                %2i           %2i           %2i\n",
			 logic_core,
			 em_core_id_get_physical(logic_core),
			 logic_to_thr_core_id(logic_core));
	}

	EM_PRINT("\n");
}

void print_cpu_arch_info(void)
{
	odp_system_info_t sysinfo;
	int err;

	err = odp_system_info(&sysinfo);
	if (err) {
		EM_PRINT("%s(): odp_system_info() call failed:%d\n",
			 __func__, err);
		return;
	}

	/* detect & print EM SW ISA info here also */
	odp_cpu_arch_isa_t isa_em = detect_sw_isa(sysinfo.cpu_arch);

	const char *cpu_arch = cpu_arch_name(sysinfo.cpu_arch);
	const char *hw_isa = cpu_arch_isa_name(sysinfo.cpu_arch,
					       sysinfo.cpu_isa_hw);
	const char *sw_isa_odp = cpu_arch_isa_name(sysinfo.cpu_arch,
						   sysinfo.cpu_isa_sw);
	const char *sw_isa_em = cpu_arch_isa_name(sysinfo.cpu_arch, isa_em);

	EM_PRINT("CPU model:              %s\n"
		 "CPU arch:               %s\n"
		 "CPU ISA version:        %s\n"
		 " SW ISA version (ODP):  %s\n"
		 " SW ISA version (EM):   %s\n",
		 odp_cpu_model_str(), cpu_arch,
		 hw_isa, sw_isa_odp, sw_isa_em);
}

static void print_odp_info(void)
{
	EM_PRINT("ODP API version:        %s\n"
		 "ODP impl name:          %s\n"
		 "ODP impl details:       %s\n",
		 odp_version_api_str(),
		 odp_version_impl_name(),
		 odp_version_impl_str());
}

static void print_mem_info(void)
{
	EM_PRINT("EM shared mem size:     %zu B\n"
		 "EM local mem size:       %zu B (per core)\n",
		 sizeof(em_shm_t), sizeof(em_locm_t));

	EM_DBG("\nem_locm_t:\t\t\t offset\t   size\n"
	       "\t\t\t\t ------\t   ----\n"
	       "current:\t\t\t%5zu B\t%5zu B\n"
	       "idle_state:\t\t\t%5zu B\t%5zu B\n"
	       "core_id:\t\t\t%5zu B\t%5zu B\n"
	       "event_burst_cnt:\t\t%5zu B\t%5zu B\n"
	       "atomic_group_released:\t\t%5zu B\t%5zu B\n"
	       "do_input_poll:\t\t\t%5zu B\t%5zu B\n"
	       "do_output_drain:\t\t%5zu B\t%5zu B\n"
	       "is_external_thr:\t\t%5zu B\t%5zu B\n"
	       "is_sched_paused:\t\t%5zu B\t%5zu B\n"
	       "dispatch_cnt:\t\t\t%5zu B\t%5zu B\n"
	       "dispatch_last_run:\t\t%5zu B\t%5zu B\n"
	       "poll_drain_dispatch_cnt:\t%5zu B\t%5zu B\n"
	       "poll_drain_dispatch_last_run:\t%5zu B\t%5zu B\n"
	       "local_queues:\t\t\t%5zu B\t%5zu B\n"
	       "start_eo_elem:\t\t\t%5zu B\t%5zu B\n"
	       "error_count:\t\t\t%5zu B\t%5zu B\n"
	       "log_fn:\t\t\t\t%5zu B\t%5zu B\n"
	       "sync_api:\t\t\t%5zu B\t%5zu B\n"
	       "debug_ts[]:\t\t\t%5zu B\t%5zu B\n"
	       "output_queue_track:\t\t%5zu B\t%5zu B\n"
	       "end:\t\t\t\t%5zu B\t%5zu B\n\n",
	       offsetof(em_locm_t, current),
	       sizeof_field(em_locm_t, current),
	       offsetof(em_locm_t, idle_state),
	       sizeof_field(em_locm_t, idle_state),
	       offsetof(em_locm_t, core_id),
	       sizeof_field(em_locm_t, core_id),
	       offsetof(em_locm_t, event_burst_cnt),
	       sizeof_field(em_locm_t, event_burst_cnt),
	       offsetof(em_locm_t, atomic_group_released),
	       sizeof_field(em_locm_t, atomic_group_released),
	       offsetof(em_locm_t, do_input_poll),
	       sizeof_field(em_locm_t, do_input_poll),
	       offsetof(em_locm_t, do_output_drain),
	       sizeof_field(em_locm_t, do_output_drain),
	       offsetof(em_locm_t, is_external_thr),
	       sizeof_field(em_locm_t, is_external_thr),
	       offsetof(em_locm_t, is_sched_paused),
	       sizeof_field(em_locm_t, is_sched_paused),
	       offsetof(em_locm_t, dispatch_cnt),
	       sizeof_field(em_locm_t, dispatch_cnt),
	       offsetof(em_locm_t, dispatch_last_run),
	       sizeof_field(em_locm_t, dispatch_last_run),
	       offsetof(em_locm_t, poll_drain_dispatch_cnt),
	       sizeof_field(em_locm_t, poll_drain_dispatch_cnt),
	       offsetof(em_locm_t, poll_drain_dispatch_last_run),
	       sizeof_field(em_locm_t, poll_drain_dispatch_last_run),
	       offsetof(em_locm_t, local_queues),
	       sizeof_field(em_locm_t, local_queues),
	       offsetof(em_locm_t, start_eo_elem),
	       sizeof_field(em_locm_t, start_eo_elem),
	       offsetof(em_locm_t, error_count),
	       sizeof_field(em_locm_t, error_count),
	       offsetof(em_locm_t, log_fn),
	       sizeof_field(em_locm_t, log_fn),
	       offsetof(em_locm_t, sync_api),
	       sizeof_field(em_locm_t, sync_api),
	       offsetof(em_locm_t, debug_ts),
	       sizeof_field(em_locm_t, debug_ts),
	       offsetof(em_locm_t, output_queue_track),
	       sizeof_field(em_locm_t, output_queue_track),
	       offsetof(em_locm_t, end),
	       sizeof_field(em_locm_t, end)
	);
}

/**
 * Print information about EM & the environment
 */
void print_em_info(void)
{
	EM_PRINT("\n"
		 "===========================================================\n"
		 "EM Info on target: %s\n"
		 "===========================================================\n"
		 "EM API version:         %s\n"
		 "EM version:             %s, "
#ifdef EM_64_BIT
		 "64 bit "
#else
		 "32 bit "
#endif
		 "(EM_CHECK_LEVEL:%d, EM_ESV_ENABLE:%d)\n"
		 "EM build info:          %s\n",
		 EM_TARGET_STR, EM_VERSION_API_STR, EM_VERSION_STR,
		 EM_CHECK_LEVEL, EM_ESV_ENABLE,
		 EM_BUILD_INFO_STR);

	print_odp_info();
	print_cpu_arch_info();
	print_mem_info();
	print_core_map_info();
	print_queue_capa();
	print_queue_elem_info();
	queue_group_info_print_all();
	em_pool_info_print_all();
	print_pool_elem_info();
	print_ag_elem_info();
	print_event_info();
}
