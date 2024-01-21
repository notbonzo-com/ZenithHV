#include <stdint.h>
#include <stddef.h>
#include "gdt.h"

struct __attribute__((packed)) {
    segment_descriptor segments[5];
    system_segment_descriptor tss;
} g_GDT;

struct __attribute__((packed)) {
    uint16_t size;
    uintptr_t gdt_ptr;
} g_GDT_Descriptor;

void x64_GDT_Initilise(void) {
    // NULL segment starting at 0x0
    g_GDT.segments[0] = (segment_descriptor){ 0 };
    // 64 Bit kernel code segment
    g_GDT.segments[1] = (segment_descriptor) {
        .limit_low = 0x0,
        .base_low = 0x0,
        .base_mid = 0x0,
        .access_byte = 0b10011010, // Executable, R/W True
        .limit_high_and_flags = 0b10100000,
        .base_high = 0x0
    };
    // 64 Bit kernel data segment
    g_GDT.segments[2] = (segment_descriptor) {
        .limit_low = 0x0,
        .base_low = 0x0,
        .base_mid = 0x0,
        .access_byte = 0b10010010, // Not executable, R/W True
        .limit_high_and_flags = 0b10100000,
        .base_high = 0x0
    };
    // 32 Bit user code segment
    g_GDT.segments[3] = (segment_descriptor) {
        .limit_low = 0x0,
        .base_low = 0x0,
        .base_mid = 0x0,
        .access_byte = 0b11111010, // Executable, R/W True, Privilege 3
        .limit_high_and_flags = 0b10100000,
        .base_high = 0x0
    };
    // 32 Bit user data segment
    g_GDT.segments[4] = (segment_descriptor) {
        .limit_low = 0x0,
        .base_low = 0x0,
        .base_mid = 0x0,
        .access_byte = 0b11110010, // Not executable, R/W True, Privilege 3
        .limit_high_and_flags = 0b10100000,
        .base_high = 0x0
    };

    // TSS
    g_GDT.tss.descriptor.limit_low = sizeof(tss);
    g_GDT.tss.descriptor.base_low = 0;
    g_GDT.tss.descriptor.base_mid = 0;
    // Present, Accessed, Executable, R/W True, Accessed, TSS
    g_GDT.tss.descriptor.access_byte = 0b10001001;
    // Size
    g_GDT.tss.descriptor.limit_high_and_flags = 0b01000000;
    g_GDT.tss.descriptor.base_high = 0;
    g_GDT.tss.base = 0;
    g_GDT.tss.reserved = 0;

    g_GDT_Descriptor.size = (uint16_t)(sizeof(g_GDT) - 1);
    g_GDT_Descriptor.gdt_ptr = (uintptr_t)&g_GDT;

    x86_GDT_Reload();
}

void x86_GDT_TSS_Reload(uintptr_t tss_address) {
    // tss (size already set)
    g_GDT.tss.descriptor.base_low = (uint16_t)(tss_address);
    g_GDT.tss.descriptor.base_mid = (uint8_t)(tss_address >> 16);
    // Present, Accessed, Executable, R/W True, Accessed, TSS
    g_GDT.tss.descriptor.base_high = (uint8_t)(tss_address >> 24);
    g_GDT.tss.base = (uint32_t)(tss_address >> 32);

    __asm__ volatile (
        // load tss entry index: 5 * 8 byte
        "ltr %0"
        : : "rm" ((uint16_t)40) : "memory"
    );
}

void x86_GDT_Reload()
{
    __asm__ volatile (
        "mov %0, %%rdi\n"
        "lgdt (%%rdi)\n"
        "push $0x8\n" // 8: offset kernel code 64 bit
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        "mov $0x10, %%ax\n" // 10: offset kernel data 64 bit
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "retq"
        :
        : "r" (&g_GDT_Descriptor)
        : "memory"
    );
}