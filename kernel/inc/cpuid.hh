#pragma once

#include <stdint.h>

using qword = uint64_t;
using dword = uint32_t;
using word = uint16_t;
using byte = uint8_t;

struct cpuid_ctx {
    dword leaf;
    dword eax;
    dword ebx;
    dword ecx;
    dword edx;
};

struct cpuid_data_common {
    uint32_t highest_supported_std_func;
    char cpu_vendor[13];
    uint16_t family;
    uint8_t model;
    uint8_t stepping;
    uint8_t apic_id;
    uint8_t cpu_count;
    uint8_t clflush_size;
    uint8_t brand_id;
    uint32_t feature_flags_ecx;
    uint32_t feature_flags_edx;
    char cpu_name_string[49];
};

void cpuid_common(struct cpuid_data_common *data);
extern "C" bool __get_cpuid(uint32_t op, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
void cpuid(struct cpuid_ctx *ctx);