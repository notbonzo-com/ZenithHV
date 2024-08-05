#include <sys/gdt.hpp>
#include <kprintf>
#include <atomic>

namespace gdt
{

pointer_t GDTR;


struct __pack {
    descriptor_t GDTS[5];
    descriptor_ex_t TSS;
} GDT;

void init()
{
    kprintf(" -> Descriptor 0 (0x0): NULL\n");
    GDT.GDTS[0] = (descriptor_t) { 0,0,0,0,0,0 };
    kprintf(" -> Descriptor 1 (0x8): Kernel Code\n");
    GDT.GDTS[1] = (descriptor_t) {
        0, 0, 0,
        0b10011010,   // Access: Present, Ring 0, Code, Executable, Readable, not Accessed
        0b10100000, 0 // Granularity: 32-bit, 4KB granularity
    };
    kprintf("    Access Flags: Present, Ring 0, Code, Executable, Readable, not Accessed\n");
    kprintf("    Granularity Flags: 32-bit operand size, 4KB block size\n");

    kprintf(" -> Descriptor 1 (0x10): Kernel Data\n");
    GDT.GDTS[2] = (descriptor_t) {
        0, 0, 0,
        0b10010010,   // Access: Present, Ring 0, Data, Writable, not Accessed
        0b10100000, 0 // Granularity: 32-bit, 4KB granularity
    };
    kprintf("    Access Flags: Present, Ring 0, Data, Writable, not Accessed\n");
    kprintf("    Granularity Flags: 32-bit operand size, 4KB block size\n");

    kprintf(" -> Descriptor 1 (0x18): Userspace Code\n");
    GDT.GDTS[3] = (descriptor_t) {
        0, 0, 0,
        0b11111010,   // Access: Present, Ring 3, Code, Executable, Readable, not Accessed
        0b10100000, 0 // Granularity: 32-bit, 4KB granularity
    };
    kprintf("    Access Flags: Present, Ring 3, Code, Executable, Readable, not Accessed\n");
    kprintf("    Granularity Flags: 32-bit operand size, 4KB block size\n");

    kprintf(" -> Descriptor 1 (0x20): Userspace Data\n");
    GDT.GDTS[4] = (descriptor_t) {
        0, 0, 0,
        0b11110010,   // Access: Present, Ring 3, Data, Writable, not Accessed
        0b10100000, 0 // Granularity: 32-bit, 4KB granularity
    };
    kprintf("    Access Flags: Present, Ring 3, Data, Writable, not Accessed\n");
    kprintf("    Granularity Flags: 32-bit operand size, 4KB block size\n");

    GDTR.size = (uint16_t) (sizeof(GDT) - 1);
    GDTR.offset = (uintptr_t) (&GDT);

    kprintf(" -> Reloading the GDT and segment registers\n");
    reload();
    kprintf(" -> GDT initialization complete\n");
}

void reload()
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

}

namespace tss {
void init()
{
    kprintf(" -> Limit Low: %00x", sizeof(tss_t));
    ::gdt::GDT.TSS.limit_low = sizeof(tss_t);
    kprintf(" -> Base Low: 0\n");
    ::gdt::GDT.TSS.base_low = 0;
    kprintf(" -> Base Middle: 0\n");
    ::gdt::GDT.TSS.base_mid = 0;
    kprintf(" -> Base Hight: 0\n");
    ::gdt::GDT.TSS.base_high = 0;
    kprintf(" -> Acces Byte: 10001001\n");
    ::gdt::GDT.TSS.access_byte = 0b10001001;
    kprintf(" -> Limit Hight && Flags: 0\n");
    ::gdt::GDT.TSS.limit_high_and_flags = 0;
    kprintf(" -> Base: 0\n");
    ::gdt::GDT.TSS.base = 0;
    kprintf(" -> Reserved: 0\n");
    ::gdt::GDT.TSS.reserved = 0;
    kprintf(" -> TSS initialization complete\n");
}


std::klock tss_lock;

void reload(tss_t* addr)
{
    tss_lock.a();
    ::gdt::GDT.TSS.base_low = (uint16_t)((uintptr_t)addr & 0xFFFF);
    ::gdt::GDT.TSS.base_mid = (uint8_t)(((uintptr_t)addr >> 16) & 0xFF);
    ::gdt::GDT.TSS.base_high = (uint8_t)(((uintptr_t)addr >> 24) & 0xFF);
    ::gdt::GDT.TSS.limit_low = sizeof(tss_t);
    ::gdt::GDT.TSS.access_byte = 0b10001001;
    ::gdt::GDT.TSS.limit_high_and_flags = (uint8_t)(((sizeof(tss_t) >> 16) & 0x0F) | (0b0000));

    asm volatile (
        "ltr %0"
        :
        : "rm" ((uint16_t)0x28)
        : "memory"
    );
    tss_lock.r();
}

}