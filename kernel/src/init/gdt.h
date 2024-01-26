#pragma once
#include <stdint.h>

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t limit_high_and_flags;
    uint8_t base_high;
} __attribute__((packed)) segment_descriptor;

typedef struct {
    segment_descriptor descriptor;
    uint32_t base;
    uint32_t reserved;
} __attribute__((packed)) system_segment_descriptor;

typedef struct {
    uint32_t res_0;
    uint64_t rsp[3]; // stack pointers
    uint64_t res_1;
    uint64_t ist[7]; // interrupt stack tables
    uint64_t res_2;
    uint16_t res_3;
    uint16_t io_permission_bitmap;
} __attribute__((packed)) tss;

/**
 * Initializes the GDT (Global Descriptor Table) for x64 architecture.
 */
void x64_GDT_Init(void);

/**
 * Reloads the GDT for x86 architecture.
 */
void x86_GDT_Reload();

/**
 * Reloads the TSS (Task State Segment) for x86 architecture.
 * 
 * @param tss_address The address of the TSS.
 */
void x86_GDT_TSS_Reload(uintptr_t tss_address);