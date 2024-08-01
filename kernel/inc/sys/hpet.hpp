#pragma once

#include <sys/apci.hpp>
#include <stdint.h>

namespace hpet {

typedef struct {
    apci::SDTHeader header;
    uint8_t hardware_rev_id;
    uint8_t info;
    uint16_t pci_id;
    uint8_t address_space_id;
    uint8_t register_width;
    uint8_t register_offset;
    uint8_t reserved;
    uint64_t addr;
    uint8_t hpet_num;
    uint16_t minimum_ticks;
    uint8_t page_protection;
} __attribute__((packed)) header;

typedef struct {
    uint64_t capabilities;
    uint64_t reserved0;
    uint64_t general_config;
    uint64_t reserved1;
    uint64_t int_status;
    uint64_t reserved2;
    uint64_t reserved3[24];
    volatile uint64_t counter_val;
    uint64_t unused4;
} __attribute__((packed)) regs_t;

extern regs_t *regs;

bool init();
void usleep(uint64_t us);

}