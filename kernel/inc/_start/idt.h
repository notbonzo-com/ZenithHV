#pragma once
#include <stdint.h>
#include <stddef.h>
#include <util.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t attribute;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __pack idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t offset;
} __pack idt_pointer_t;

typedef struct {
    uint64_t es, ds;
    uint64_t CR4, CR3, CR2, CR0;
    uint64_t R15, R14, R13, R12, R11, R10, R9, R8, RAX, RBX, RCX, RDX, RBP, RSI, RDI;
    uint64_t interrupt, error;
    uint64_t rip, cs, rflags, rsp, ss;                   // pushed automatically by CPU
} __pack regs_t;

extern __attribute__((aligned(16))) idt_entry_t entries[256];
extern uintptr_t handlers[256];
extern uint64_t stubs[];

void idt_load();
void idt_init();

void kpanic(regs_t *regs, const char* str);
void stacktrace(regs_t *regs);
void idt_registerVector(size_t vector, uintptr_t handler);
void idt_eraseVector(size_t vector);
void idt_setGate(uint8_t interrupt, uintptr_t base, int8_t flags);

#ifdef __cplusplus
}
#endif