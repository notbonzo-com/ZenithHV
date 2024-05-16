#include <_start/gdt.h>
#include <smp.h>
#include <util.h>
#include <kprintf.h>

gdt_pointer_t GDTR;

struct __pack {
    gdt_descriptor_t GDTS[5];
    gdt_descriptor_ex_t TSS;
} GDT;

void gdt_init()
{
    kprintf(" -> Descriptor 0 (0x0): NULL\n");
    GDT.GDTS[0] = (gdt_descriptor_t) { 0,0,0,0,0,0 };
    kprintf(" -> Descriptor 1 (0x8): Kernel Code\n");
    GDT.GDTS[1] = (gdt_descriptor_t) {
        0, 0, 0,
        0b10011010,   // Access: Present, Ring 0, Code, Executable, Readable, not Accessed
        0b10100000, 0 // Granularity: 32-bit, 4KB granularity
    };
    kprintf("    Access Flags: Present, Ring 0, Code, Executable, Readable, not Accessed\n");
    kprintf("    Granularity Flags: 32-bit operand size, 4KB block size\n");

    kprintf(" -> Descriptor 1 (0x10): Kernel Data\n");
    GDT.GDTS[2] = (gdt_descriptor_t) {
        0, 0, 0,
        0b10010010,   // Access: Present, Ring 0, Data, Writable, not Accessed
        0b10100000, 0 // Granularity: 32-bit, 4KB granularity
    };
    kprintf("    Access Flags: Present, Ring 0, Data, Writable, not Accessed\n");
    kprintf("    Granularity Flags: 32-bit operand size, 4KB block size\n");

    kprintf(" -> Descriptor 1 (0x18): Userspace Code\n");
    GDT.GDTS[3] = (gdt_descriptor_t) {
        0, 0, 0,
        0b11111010,   // Access: Present, Ring 3, Code, Executable, Readable, not Accessed
        0b10100000, 0 // Granularity: 32-bit, 4KB granularity
    };
    kprintf("    Access Flags: Present, Ring 3, Code, Executable, Readable, not Accessed\n");
    kprintf("    Granularity Flags: 32-bit operand size, 4KB block size\n");

    kprintf(" -> Descriptor 1 (0x20): Userspace Data\n");
    GDT.GDTS[4] = (gdt_descriptor_t) {
        0, 0, 0,
        0b11110010,   // Access: Present, Ring 3, Data, Writable, not Accessed
        0b10100000, 0 // Granularity: 32-bit, 4KB granularity
    };
    kprintf("    Access Flags: Present, Ring 3, Data, Writable, not Accessed\n");
    kprintf("    Granularity Flags: 32-bit operand size, 4KB block size\n");

    GDTR.size = (uint16_t) (sizeof(GDT) - 1);
    GDTR.offset = (uintptr_t) (&GDT);

    kprintf(" -> Reloading the GDT and segment registers\n");
    gdt_reload();
    kprintf(" -> GDT initialization complete");
}

void gdt_reload()
{
    kprintf(" -> Loading GDT into GDTR\n");
    asm volatile (
        "mov %0, %%rdi\n"
        "lgdt (%%rdi)\n"
        "push $0x8\n"
        "lea 1f(%%rip), %%rax\n"
        "push %%rax\n"
        "lretq\n"
        "1:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%ss\n"
        "mov %%ax, %%ds\n"
        :
        : "r" (&GDTR)
        : "memory"
    );
}

void tss_init()
{
    kprintf(" -> Limit Low: %00x", sizeof(tss_t));
    GDT.TSS.limit_low = sizeof(tss_t);
    kprintf(" -> Base Low: 0\n");
    GDT.TSS.base_low = 0;
    kprintf(" -> Base Middle: 0\n");
    GDT.TSS.base_mid = 0;
    kprintf(" -> Base Hight: 0\n");
    GDT.TSS.base_high = 0;
    kprintf(" -> Acces Byte: 10001001\n");
    GDT.TSS.access_byte = 0b10001001;
    kprintf(" -> Limit Hight && Flags: 0\n");
    GDT.TSS.limit_high_and_flags = 0;
    kprintf(" -> Base: 0\n");
    GDT.TSS.base = 0;
    kprintf(" -> Reserved: 0\n");
    GDT.TSS.reserved = 0;
    kprintf(" -> TSS initialization complete\n");
}

klock tss_lock;
void tss_reload(tss_t* addr)
{
    acquire_lock(&tss_lock);
    GDT.TSS.base_low = (uint16_t)((uintptr_t)addr & 0xFFFF);
    GDT.TSS.base_mid = (uint8_t)(((uintptr_t)addr >> 16) & 0xFF);
    GDT.TSS.base_high = (uint8_t)(((uintptr_t)addr >> 24) & 0xFF);
    GDT.TSS.limit_low = sizeof(tss_t);
    GDT.TSS.access_byte = 0b10001001;
    GDT.TSS.limit_high_and_flags = (uint8_t)(((sizeof(tss_t) >> 16) & 0x0F) | (0b0000));

    asm volatile (
        "ltr %0"
        :
        : "rm" ((uint16_t)0x28)
        : "memory"
    );
	release_lock(&tss_lock);
}