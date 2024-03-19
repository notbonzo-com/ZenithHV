/**
 * @file int.c
 * @brief This file contains the implementation of interrupt handling in the kernel.
 */

#include <mem.h>
#include "int.h"

#include <debug.h>
#include <stdarg.h>

uintptr_t g_IdtHandlers[256] = {0};

struct __attribute__((packed)) {
    uint16_t size;
    uint64_t offset;
} idtr;

__attribute__((aligned(16))) idtDesc_t idt[256];

extern uint64_t g_IsrStrubTable[];

static const char * cpuExceptionStrings[32] = {
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
    "Invalid TSS, ",
    "Segment not present, ",
    "Stack Segment Fault, ",
    "General Protection Fault, ",
    "Page Fault, ",
    "RESERVED VECTOR",
    "x87 FP Exception",
    "Alignment Check, ",
    "Machine Check (Internal Error)",
    "SIMD FP Exception",
    "Virtualization Exception",
    "Control  Protection Exception, ",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "RESERVED VECTOR",
    "Hypervisor Injection Exception",
    "VMM Communication Exception, ",
    "Security Exception, ",
    "RESERVED VECTOR"
};

void dumpRegs(intRegInfo_t *regs) {
    // Log interrupt vector and exception string
    log_crit("Interrupt", 
             "\n[IV 0x%lX] -> %s", 
             regs->vector, cpuExceptionStrings[regs->vector]);

    // Log error code for specific vectors
    if (regs->vector == 8 || regs->vector == 10 || regs->vector == 11 || 
        regs->vector == 12 || regs->vector == 13 || regs->vector == 14 || 
        regs->vector == 17 || regs->vector == 30) {
        log_crit("Interrupt", 
                 "[ec 0x%lX]:\n\r", 
                 regs->error_code);
    }

    // Log general purpose registers
    log_crit("INT", 
             "rdi: 0x%p   rsi: 0x%p   rbp: 0x%p   rsp: 0x%p\n"
             "rbx: 0x%p   rdx: 0x%p   rcx: 0x%p   rax: 0x%p",
             regs->rdi, regs->rsi, regs->rbp, regs->rsp,
             regs->rbx, regs->rdx, regs->rcx, regs->rax);

    // Log extended registers
    log_crit("INT", 
             "r8:  0x%p   r9:  0x%p   r10: 0x%p   r11: 0x%p\n"
             "r12: 0x%p   r13: 0x%p   r14: 0x%p   r15: 0x%p",
             regs->r8, regs->r9, regs->r10, regs->r11,
             regs->r12, regs->r13, regs->r14, regs->r15);

    // Log control registers
    log_crit("INT", 
             "cr0: 0x%p   cr2: 0x%p   cr3: 0x%p   cr4: 0x%p",
             regs->cr0, regs->cr2, regs->cr3, regs->cr4);

    // Log EFLAGS
    log_crit("INT", "RFLAGS: 0x%b\n\n", regs->rflags);
}


void defaultExceptionHandler(intRegInfo_t *regs)
{
    panic(regs, 0, "Unhandeled Expection\n");
    return;
}

void panic(intRegInfo_t *regs, uint8_t quiet, const char *format, ...)
{
    (void)quiet;

    if (regs) {
        dumpRegs(regs);
    }

    va_list args;
    va_start(args, format);
    log_err("Kernel Panic", format, args);
    va_end(args);

    __asm__ ("cli\n hlt");
}

void empty_handler(intRegInfo_t *regs)
{
    int intNumber = regs->vector;
    log_warn("Interrupt", "Unhandled interrupt occured: %x \n", intNumber);
    __asm__ ("hlt");
}

void setVector(uint8_t vector, uintptr_t isr, uint8_t flags) {
    idtDesc_t *descriptor = &idt[vector];

    descriptor->offset_low = isr & 0xFFFF;
    descriptor->selector = 0x8; // 8: kernel code selector offset (gdt)
    descriptor->ist = 0;
    descriptor->type_attributes = flags;
    descriptor->offset_mid = (isr >> 16) & 0xFFFF;
    descriptor->offset_high = (isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}

int initIdt(void)
{
    for (size_t vector = 0; vector < 256; vector++) {
        setVector(vector, g_IsrStrubTable[vector], 0x8E);
    }
    for (size_t vector = 0; vector < 32; vector++) {
        g_IdtHandlers[vector] = (uintptr_t)defaultExceptionHandler;
    }
    for (size_t vector = 32; vector < 256; vector++) {
        g_IdtHandlers[vector] = (uintptr_t)empty_handler;
    }

    loadIdt();
    return 0;
}

bool registerVector(size_t vector, uintptr_t handler)
{
    if (g_IdtHandlers[vector] != (uintptr_t)empty_handler || !handler) {
        log_crit("Interrupt", "Failed to register vector %lu at 0x%p\n", vector, handler);
        return false;
    }
    g_IdtHandlers[vector] = handler;
    return true;
}

bool resetVector(size_t vector)
{
    if (g_IdtHandlers[vector] == (uintptr_t)empty_handler || vector < 32) {
        log_crit("Interrupt", "Failed to erase vector %lu\n", vector);
        return false;
    }
    g_IdtHandlers[vector] = (uintptr_t)empty_handler;
    return true;
}

inline void loadIdt(void)
{
    idtr.offset = (uintptr_t)idt;
    idtr.size = (uint16_t)(sizeof(idt) - 1);

    __asm__ volatile (
        "lidt %0"
        : : "m"(idtr) : "memory"
    );
}

void sendEoi(void)
{
    __asm__ volatile (
        "movb $0x20, %%al\n"
        "outb %%al, $0x20\n"
        "outb %%al, $0xA0\n"
        : : : "memory"
    );
}