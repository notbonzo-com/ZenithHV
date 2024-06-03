#pragma once

#include <stdint.h>
#include <stddef.h>
#include <util>

namespace intr {

typedef struct {
    uint64_t es, ds;
    uint64_t CR4, CR3, CR2, CR0;
    uint64_t R15, R14, R13, R12, R11, R10, R9, R8, RAX, RBX, RCX, RDX, RBP, RSI, RDI;
    uint64_t interrupt, error;
    uint64_t rip, cs, rflags, rsp, ss;                   // pushed automatically by CPU
} __pack regs_t;

using handler_t = void (*)(regs_t* regs);

void kpanic(regs_t *regs, const char* str);
void registerVector(size_t vector, uintptr_t handler);
void eraseVector(size_t vector);
void setGate(uint8_t interrupt, uintptr_t base, int8_t flags);
void stacktrace(regs_t *regs);
extern "C" void default_interrupt_handler(regs_t* CPU);

}