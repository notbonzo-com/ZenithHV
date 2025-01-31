//
// Created by notbonzo on 1/30/25.
//

#include <arch/x86_64/gdt.h>
#include <common/constants.h>
#include <arch/x86_64/common.h>
#include <common/printf.h>

#define GDT_ENTRIES_COUNT 3

struct gdt_descriptor gdt_descriptor[GDT_ENTRIES_COUNT] = {
    { 0,0,0,0,0,0 },
    { 0, 0, 0, 0b10011010, 0b10100000, 0 }, /* Access: Present, Ring 0, Code, Executable, Readable, not Accessed
                                            Granularity: 32-bit, 4KB granularity */
    { 0, 0, 0, 0b10010010, 0b10100000, 0 } /* Access: Present, Ring 0, Data, Writable, not Accessed
                                            Granularity: 32-bit, 4KB granularity */
};
struct table_pointer gdt_pointer = { (uint16_t)( sizeof( gdt_descriptor ) - 1 ), (uintptr_t)&gdt_descriptor };

void init_gdt( )
{
#ifdef DEBUG
    printf("\nGlobal Descriptor Table (GDT)\n");
    printf("============================================\n");
    printf("|  #  | Base     | Limit  | Access | Flags |\n");
    printf("============================================\n");

    for (int i = 0; i < GDT_ENTRIES_COUNT; i++) {
        struct gdt_descriptor* entry = &gdt_descriptor[i];

        uint32_t base = (entry->base_high << 24) | (entry->base_mid << 16) | entry->base_low;
        uint32_t limit = ((entry->limit_high_and_flags & 0x0F) << 16) | entry->limit_low;
        uint8_t flags = (entry->limit_high_and_flags & 0xF0) >> 4;

        printf("| %2d  | %08x | %05x |  0x%02x   |  0x%01x  |\n",
               i, base, limit, entry->access_byte, flags);
    }

    printf("============================================\n");
#endif
    __asm__ volatile (
            "lgdt (%%rax)\n"
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
            : "a"(&gdt_pointer)
            : "memory"
    );
}
