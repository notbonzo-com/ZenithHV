#pragma once
#include <stdint.h>

namespace gdt {

    typedef struct {
        uint16_t limit_low;
        uint16_t base_low;
        uint8_t base_mid;
        uint8_t access_byte;
        uint8_t limit_high_and_flags;
        uint8_t base_high;
    } __attribute__((packed)) descriptor_t;

    typedef struct {
        uint16_t size;
        uintptr_t offset;
    } __attribute__((packed)) pointer_t;

    void init();
    void reload();
    extern pointer_t GDTR;

}