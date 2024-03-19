#pragma once
#include <stdint.h>

#define PIC1_BASE                   0x20
#define PIC2_BASE                   0xA0
#define PIC1_COMMAND                0x20
#define PIC2_COMMAND                0xA0
#define PIC1_DATA                   0x21
#define PIC2_DATA                   0xA1

#define PIC_MASTER_OFFSET           0x20
#define PIC_SLAVE_OFFSET            0x28

#define IA32_APIC_BASE_MSR          0x1B
#define IA32_APIC_BASE_MSR_BSP      0x100
#define IA32_APIC_BASE_MSR_ENABLE   0x800
#define APIC_EOI                    0xB0

void initApic(void);