//
// Created by notbonzo on 1/30/25.
//

#ifndef GDT_H
#define GDT_H

#include <stdint.h>

struct [[gnu::packed]] gdt_descriptor {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access_byte;
    uint8_t limit_high_and_flags;
    uint8_t base_high;
};

void init_gdt( );

#endif //GDT_H
