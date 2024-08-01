#pragma once
#include <stdint.h>
#include <util>

namespace gdt {

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t limit_high_and_flags;
    uint8_t base_high;
} __pack descriptor_t;

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t limit_high_and_flags;
    uint8_t base_high;
    uint32_t base;
    uint32_t reserved;
} __pack descriptor_ex_t;

typedef struct {
    uint16_t size;
    uintptr_t offset;
} __pack pointer_t;

void init();
void reload();
extern pointer_t GDTR;
}

namespace tss {
void init();

typedef struct {
    uint32_t reserved_0;
    uint64_t rsp0;  // privilege level stacks
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved_1;
    uint64_t ist1;  // additional stack (IDT IST)
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved_2;
    uint16_t reserved_3;
    uint16_t iomba;
} tss_t;

void reload(tss_t* addr);
}
