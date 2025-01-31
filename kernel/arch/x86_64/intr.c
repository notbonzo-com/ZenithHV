//
// Created by notbonzo on 1/31/25.
//

#include <arch/x86_64/intr.h>
#include <common/constants.h>
#include <arch/x86_64/common.h>
#include <arch/io.h>
#include <smp/lock.h>
#include <common/printf.h>
#include <string.h>

[[gnu::aligned(16)]] struct idt_entry idt_descriptor[256] = {0};

idt_intr_handler realHandler[256];
extern uintptr_t stubs[];

struct table_pointer idt_pointer = { (uint16_t)( sizeof( idt_descriptor ) - 1 ), (uintptr_t)&idt_descriptor };

static const char* strings[32] = {
    "Division by Zero",
    "Debug",
    "Non-Maskable-Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid opcode",
    "Device (FPU) not available",
    "Double Fault",
    "RESERVED VECTOR",
    "Invalid TSS",
    "Segment not present",
    "Stack Segment Fault",
    "General Protection Fault ",
    "Page Fault ",
    "RESERVED VECTOR",
    "x87 FP Exception",
    "Alignment Check",
    "Machine Check (Internal Error)",
    "SIMD FP Exception",
    "Virtualization Exception",
    "Control  Protection Exception",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "RESERVED VECTOR"
};

[[noreturn]] void idt_default_interrupt_handler( struct registers_ctx* ctx )
{
    kpanic( ctx, nullptr );
}

#include <common/kpanic.inl>

#define SET_GATE(interrupt, base, flags) do { \
    idt_descriptor[(interrupt)].offset_low = (base) & 0xFFFF; \
    idt_descriptor[(interrupt)].selector = 0x8; \
    idt_descriptor[(interrupt)].ist = 0; \
    idt_descriptor[(interrupt)].attribute = (flags); \
    idt_descriptor[(interrupt)].offset_mid = ((base) >> 16) & 0xFFFF; \
    idt_descriptor[(interrupt)].offset_high = ((base) >> 32) & 0xFFFFFFFF; \
    idt_descriptor[(interrupt)].zero = 0; \
} while(0)

static irq_lock_t register_handler_lock;

void init_idt( )
{
    irq_lock_init(&register_handler_lock);
    for ( size_t vector = 0; vector < 32; vector++ ) {
        SET_GATE( vector, stubs[vector], 0b10001111 );
        realHandler[vector] = (idt_default_interrupt_handler);
    }
    for ( size_t vector = 32; vector < 256; vector++ ) {
        SET_GATE( vector, stubs[vector], 0b10001110 );
        realHandler[vector] = (idt_default_interrupt_handler);
    }

#ifdef DEBUG
    printf("\nInterrupt Descriptor Table (IDT)\n");
    printf("======================================================================================\n");
    printf("| Vec | Offset             | Selector  | Type | IST | Exception Name                 |\n");
    printf("======================================================================================\n");

    for (size_t i = 0; i < 256; i++) {
        struct idt_entry *entry = &idt_descriptor[i];

        uint64_t offset = ((uint64_t)entry->offset_high << 32) | ((uint64_t)entry->offset_mid << 16) | entry->offset_low;
        const char *exception_name = (i < 32) ? strings[i] : "IRQ/Custom";

        printf("| %3lu | 0x%016llx | 0x%04x    | 0x%02x |  %d  | %-30s |\n",
               i, offset, entry->selector, entry->attribute, entry->ist, exception_name);
    }

    printf("======================================================================================\n");
#endif
}

void load_idt( )
{
    __asm__ volatile (
        "lidt %0"
        : : "m"(idt_pointer) : "memory"
    );
}

bool register_handler( size_t vector, idt_intr_handler handler )
{
    irq_lock( &register_handler_lock );
    if (realHandler[vector] == idt_default_interrupt_handler) {
        realHandler[vector] = handler;
        irq_unlock( &register_handler_lock );
#ifdef DEBUG
        printf("[DEBUG] Registered handler for interrupt %lu (0x%02lx)\n", vector, vector);
#endif
        return true;
    }

#ifdef DEBUG
    printf("[WARNING] Attempted to register handler for interrupt %lu (0x%02lx), but a handler already exists!\n",
           vector, vector);
#endif

    irq_unlock( &register_handler_lock );
    return false;
}

