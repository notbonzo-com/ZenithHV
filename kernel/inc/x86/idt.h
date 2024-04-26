#pragma once

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IDTEntry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_hi;
    uint32_t zero2;
} __attribute__((packed)) IDTEntry_t;

typedef struct IDTPointer {
    uint16_t size;
    uint64_t offset;
} __attribute__((packed)) IDTPointer_t;

typedef struct __attribute__((packed)) {
    uint64_t es;
    uint64_t ds;
    uint64_t cr4;
    uint64_t cr3;
    uint64_t cr2;
    uint64_t cr0;
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rbp;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t vector;

    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) IDTContext_t;

void default_exception_handler(IDTContext_t *regs);

void IDT_SetDescriptor(uint8_t vector, uintptr_t isr, uint8_t flags);
void initIDT();

void IDT_SetVector(size_t vector, uintptr_t handler);
void IDT_ResetVector(size_t vector);

void kpanic();

#ifdef __cplusplus
}
#endif