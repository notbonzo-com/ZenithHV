#include <stdint.h>
#include <gdt.h>
#include <mem.h>

struct __attribute__((packed)) {
    segmentDescriptor gdts[5];
    systemSegmentDescriptor tss;
} gdt;

struct __attribute__((packed)) {
    uint16_t size;
    uintptr_t gdtPtr;
} gdtr;

int initGdt(void)
{
    // null descriptor
    gdt.gdts[0] = (segmentDescriptor){ 0 };

    // 64 bit kernel code segment
    gdt.gdts[1] = (segmentDescriptor){
        0, 0, 0,
        0b10011010,
        0b10100000, 0
    };
    // 64 bit kernel data segment
    gdt.gdts[2] = (segmentDescriptor){
        0, 0, 0,
        0b10010010,
        0b10100000, 0
    };
    // 64 bit user code segment
    gdt.gdts[3] = (segmentDescriptor){
        0, 0, 0,
        0b11111010,
        0b10100000, 0
    };
    // 64 bit user data segment
    gdt.gdts[4] = (segmentDescriptor){
        0, 0, 0,
        0b11110010,
        0b10100000, 0
    };

    gdt.tss.descriptor.limitLow = sizeof(tss);
    gdt.tss.descriptor.baseLow = 0;
    gdt.tss.descriptor.baseMid = 0;
    gdt.tss.descriptor.accessByte = 0b10001001;
    gdt.tss.descriptor.limitHighAndFlags = 0;
    gdt.tss.descriptor.baseHigh = 0;
    gdt.tss.base = 0;
    gdt.tss.reserved = 0;

    gdtr.size = (uint16_t)(sizeof(gdt) - 1);
    gdtr.gdtPtr = (uintptr_t)&gdt;

    reloadGdt();

    // Check for invalid values
    if (gdtr.size == 0)
    {
        return -1;
    }
    if (gdtr.gdtPtr == 0)
    {
        return -1;
    }
    if (gdt.gdts[0].baseLow != 0)
    {
        return -1;
    }
    if (gdt.gdts[0].baseMid != 0)
    {
        return -1;
    }
    if (gdt.gdts[0].baseHigh != 0)
    {
        return -1;
    }
    if (gdt.gdts[0].limitHighAndFlags != 0)
    {
        return -1;
    }

    return 0;
}

void reloadTss(struct taskStateSegment *tssAddress)
{
    gdt.tss.base = (uint32_t)((uintptr_t)tssAddress >> 32);
    gdt.tss.descriptor.limitLow = sizeof(tss);
    gdt.tss.descriptor.baseLow = (uint16_t)((uintptr_t)tssAddress);
    gdt.tss.descriptor.baseMid = (uint8_t)((uintptr_t)tssAddress >> 16);
    gdt.tss.descriptor.accessByte = 0b10001001;
    gdt.tss.descriptor.limitHighAndFlags = 0;
    gdt.tss.descriptor.baseHigh = (uint8_t)((uintptr_t)tssAddress >> 24);
    gdt.tss.base = 0;
    gdt.tss.reserved = 0;

    __asm__ volatile (
        "ltr %0"
        : : "rm" ((uint16_t)40) : "memory"
    );
}

void reloadGdt()
{
    __asm__ volatile (
        "mov %0, %%rdi\n"
        "lgdt (%%rdi)\n"
        "push $0x8\n"
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "retq"
        :
        : "r" (&gdtr)
        : "memory"
    );
}
