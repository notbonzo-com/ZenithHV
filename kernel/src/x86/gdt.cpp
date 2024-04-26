#include <x86/gdt.h>
#include <x86/smp.h>

#define GDT_CODE_SEGMENT_KERNEL     0x08
#define GDT_DATA_SEGMENT_KERNEL     0x10
#define GDT_CODE_SEGMENT_USER       0x18
#define GDT_DATA_SEGMENT_USER       0x20
#define GDT_TSS_SEGMENT             0x28

#define GDT_LIMIT_LOW(limit)                (limit & 0xFFFF)
#define GDT_BASE_LOW(base)                  (base & 0xFFFF)
#define GDT_BASE_MIDDLE(base)               ((base >> 16) & 0xFF)
#define GDT_FLAGS_LIMIT_HI(limit, flags)    (((limit >> 16) & 0xF) | (flags & 0xF0))
#define GDT_BASE_HIGH(base)                 ((base >> 24) & 0xFF)

#define GDT_ENTRY(base, limit, access, flags) {                     \
    GDT_LIMIT_LOW(limit),                                           \
    GDT_BASE_LOW(base),                                             \
    GDT_BASE_MIDDLE(base),                                          \
    access,                                                         \
    GDT_FLAGS_LIMIT_HI(limit, flags),                               \
    GDT_BASE_HIGH(base)                                             \
}

struct __attribute__((packed)) {
    GDTEntry_t entries[5];
    TSSDescriptor_t tss;
} gdt;

GDTPointer_t gdptr;


extern "C" void initGDT() {
    gdt.entries[0] = (GDTEntry_t) { 0, 0, 0, 0, 0, 0 };

    gdt.entries[1] = (GDTEntry_t)GDT_ENTRY(0, 0, 0b10011010, 0b10100000);

    gdt.entries[2] = (GDTEntry_t)GDT_ENTRY(0, 0, 0b10010010, 0b10100000);

    gdt.entries[3] = (GDTEntry_t)GDT_ENTRY(0, 0, 0b11111010, 0b10100000);

    gdt.entries[4] = (GDTEntry_t)GDT_ENTRY(0, 0, 0b11110010, 0b10100000);

    gdt.tss.descriptor.limit_low = sizeof(TSS_t);
    gdt.tss.descriptor.base_low = 0;
    gdt.tss.descriptor.base_mid = 0;
    gdt.tss.descriptor.access_byte = 0b10001001;
    gdt.tss.descriptor.limit_high_and_flags = 0;
    gdt.tss.descriptor.base_high = 0;
    gdt.tss.base = 0;
    gdt.tss.reserved = 0;

    gdptr.limit = sizeof(gdt) - 1;
    gdptr.base = (uintptr_t)&gdt;

    reloadGDT();
}

extern "C" void reloadGDT() {
    asm volatile (
        "mov %0, %%rdi\n"
        "lgdt (%%rdi)\n"
        "push $0x8\n"
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%ss\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%fs\n"
        :
        : "r" (&gdptr)
        : "memory"
    );
}

klock* tssLock;
extern "C" void realodTSS(uintptr_t tssPointer)
{
    tssLock->acquire();
    gdt.tss.base = (uint32_t)((uintptr_t)tssPointer >> 32);
    gdt.tss.descriptor.limit_low = sizeof(TSS_t);
    gdt.tss.descriptor.base_low = (uint16_t)((uintptr_t)tssPointer);
    gdt.tss.descriptor.base_mid = (uint8_t)((uintptr_t)tssPointer >> 16);
    gdt.tss.descriptor.access_byte = 0b10001001;
    gdt.tss.descriptor.limit_high_and_flags = 0;
    gdt.tss.descriptor.base_high = (uint8_t)((uintptr_t)tssPointer >> 24);
    gdt.tss.base = 0;
    gdt.tss.reserved = 0;

     __asm__ volatile (
        "ltr %0"
        : : "rm" ((uint16_t)40) : "memory"
    );
    
    tssLock->release();
}