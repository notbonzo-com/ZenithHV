#pragma once

#include <stdint.h>
#include <stddef.h>
#include <util>

namespace intr
{

enum class IDT_FLAGS : uint8_t {
    GATE_TASK        = 0x5,
    GATE_64BIT_INT   = 0xE,
    GATE_64BIT_TRAP  = 0xF,

    RING0            = (0 << 5),
    RING1            = (1 << 5),
    RING2            = (2 << 5),
    RING3            = (3 << 5),

    PRESENT          = 0x80
};

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attribute;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __pack entry_t;

typedef struct {
    uint16_t limit;
    uint64_t offset;
} __pack pointer_t;

extern __attribute__((aligned(16))) entry_t entries[256];
extern "C" uintptr_t handlers[256];
extern "C" uint64_t stubs[];

void load();
void init();

}