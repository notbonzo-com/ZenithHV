#pragma once
#include <stdint.h>
#include <stddef.h>
#include <util.h>

/* Stripped down version of _start/idt.h for kpanic and setting vectors */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t es, ds;
    uint64_t CR4, CR3, CR2, CR0;
    uint64_t R15, R14, R13, R12, R11, R10, R9, R8, RAX, RBX, RCX, RDX, RBP, RSI, RDI;
    uint64_t interrupt, error;
    uint64_t rip, cs, rflags, rsp, ss;                   // pushed automatically by CPU
} __pack regs_t;

void kpanic(regs_t *regs, const char* str);
void idt_registerVector(size_t vector, uintptr_t handler);
void idt_eraseVector(size_t vector);

#ifdef __cplusplus
}
#endif