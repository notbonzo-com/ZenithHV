#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct GDTEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t limit_high_and_flags;
    uint8_t base_high;
} __attribute__((packed)) GDTEntry_t;

typedef struct TSS {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t io_map;
} __attribute__((packed)) TSS_t;

typedef struct TSSDescriptor {
    GDTEntry_t descriptor;
    uint32_t base;
    uint32_t reserved;
}__attribute__((packed)) TSSDescriptor_t;

typedef struct GDTPointer {
    uint16_t limit;
    uintptr_t base;
} __attribute__((packed)) GDTPointer_t;

void initGDT();
void reloadGDT();
void realodTSS(uintptr_t tssPointer);

#ifdef __cplusplus
}
#endif