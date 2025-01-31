//
// Created by notbonzo on 1/31/25.
//

#ifndef IDT_H
#define IDT_H

#include <stdint.h>
#include <stddef.h>

struct [[gnu::packed]] registers_ctx {
    uint64_t es, ds;
    uint64_t cr4, cr3, cr2, cr0;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rax, rbx, rcx, rdx, rsi, rdi;
    uint64_t rbp;
    uint64_t interrupt_vector, error_code;
    uint64_t rip, cs, rflags, rsp, ss; /* CPU automatically pushes */
};

struct [[gnu::packed]] idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attribute;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
};

typedef void(*idt_intr_handler)( struct registers_ctx* ctx );

#define IDT_INTERRUPT_GATE (0b10001110)
#define IDT_TRAP_GATE      (0b10001111)

void init_idt( );
void load_idt( );
[[noreturn]] void kpanic( struct registers_ctx* ctx, const char* fmt, ... );
bool idt_register_handler( size_t vector, idt_intr_handler handler );

[[noreturn]] void idt_default_interrupt_handler( struct registers_ctx* ctx );

#endif //IDT_H
