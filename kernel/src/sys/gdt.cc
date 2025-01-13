#include <sys/gdt.hh>
#include <kprintf>

namespace gdt
{

    pointer_t GDTR;

    descriptor_t GDT[3];

    void init()
    {
        kprintf(" -> Descriptor 0 (0x0): nullptr\n");
        GDT[0] = (descriptor_t) { 0,0,0,0,0,0 };
        kprintf(" -> Descriptor 1 (0x8): Kernel Code\n");
        GDT[1] = (descriptor_t) {
            0, 0, 0,
            0b10011010,   // Access: Present, Ring 0, Code, Executable, Readable, not Accessed
            0b10100000, 0 // Granularity: 32-bit, 4KB granularity
        };
        kprintf("    Access Flags: Present, Ring 0, Code, Executable, Readable, not Accessed\n");
        kprintf("    Granularity Flags: 32-bit operand size, 4KB block size\n");

        kprintf(" -> Descriptor 1 (0x10): Kernel Data\n");
        GDT[2] = (descriptor_t) {
            0, 0, 0,
            0b10010010,   // Access: Present, Ring 0, Data, Writable, not Accessed
            0b10100000, 0 // Granularity: 32-bit, 4KB granularity
        };
        kprintf("    Access Flags: Present, Ring 0, Data, Writable, not Accessed\n");
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