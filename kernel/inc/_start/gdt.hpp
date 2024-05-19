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
extern pointer_t GDTR;
}

namespace tss {
void init();
}
