#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

struct task_state_segment {
    uint32_t reserved_0;
    uint64_t rsp[3];
    uint64_t reserved_1;
    uint64_t ist[7];
    uint64_t reserved_2;
    uint16_t reserved_3;
    uint16_t iomba;
};

typedef struct cpu_local_t {
    size_t id;          // core id
    uint32_t lapic_id;  // lapic id of the processor
    uint64_t lapic_clock_frequency;
    struct task_state_segment tss;
    struct thread_t *idle_thread;
} cpu_local_t;

void cpuid_common(struct cpuid_data_common *data);
void cpuid_compatibility_check(struct cpuid_data_common *data);

static inline uint64_t read_msr(uint32_t reg)
{
    uint32_t eax = 0, edx = 0;
   __asm__ volatile(
        "rdmsr"
        : "=a"(eax), "=d"(edx)
        : "c"(reg)
        :"memory"
    );
   return ((uint64_t)eax | (uint64_t)edx << 32);
}

static inline void write_msr(uint32_t reg, uint64_t value)
{
    __asm__ volatile(
        "wrmsr"
        :
        : "a"((uint32_t)value), "d"((uint32_t)(value >> 32)), "c"(reg)
        : "memory"
    );
}

static inline void write_fs_base(uintptr_t address) {
    write_msr(0xc0000100, address);
}

static inline uintptr_t read_fs_base(void) {
    return read_msr(0xc0000100);
}

static inline void write_gs_base(struct thread_t *address) {
    write_msr(0xc0000101, (uintptr_t)address);
}

static inline struct thread_t *read_gs_base(void) {
    return (struct thread_t *)read_msr(0xc0000101);
}

static inline void write_kernel_gs_base(struct thread_t *address) {
    write_msr(0xc0000102, (uintptr_t)address);
}

static inline struct thread_t *read_kernel_gs_base(void) {
    return (struct thread_t *)read_msr(0xc0000102);
}

static inline void disable_interrupts(void) {
    __asm__ ("cli");
}

static inline void enable_interrupts(void) {
    __asm__ ("sti");
}

static inline bool interrupts_enabled() {
    uint64_t rflags;
    __asm__ ("pushfq; pop %0" : "=r"(rflags));
    return (rflags & 0x200) != 0;
}
