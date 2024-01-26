/**
 * @file int.c
 * @brief Implementation of interrupt handling functions.
 *
 * This file contains the implementation of interrupt handling functions, including
 * default exception handler and CPU exception panic. It also defines the CPU exception
 * strings and provides a function to dump register information during an interrupt.
 */
#include <mem.h>
#include "int.h"

#include <drivers/debug/e9.h>
#include <stdarg.h>

/**
 * @file int.c
 * @brief This file contains the implementation of interrupt handling in the kernel.
 */

uintptr_t g_IDT_handlers[256] = {0};

struct __attribute__((packed)) {
    uint16_t size;
    uint64_t offset;
} idtr;

__attribute__((aligned(16))) g_IDT_Descriptor idt[256];

extern uint64_t isr_stub_table[];

static const char * cpu_exception_strings[32] = {
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
/**
 * @brief Default exception handler function.
 * 
 * This function is called when an unhandled exception occurs.
 * It panics the system and prints an error message.
 * 
 * @param regs Pointer to the interrupt register information.
 */
void default_exception_handler(INT_REG_INFO *regs)
{
    x86_Panic(regs, 0, "Unhandeled Expection\n");
    return;
}

/**
 * @brief Dump the register information when a CPU exception occurs.
 * 
 * This function logs critical information about the CPU exception,
 * including the interrupt vector, error code (if applicable), and register values.
 * 
 * @param regs Pointer to the interrupt register information.
 */
void x86_DumpRegs(INT_REG_INFO *regs)
{
    log_crit("Interrupt", "\n[IV 0x%lX] -> %s", regs->vector, cpu_exception_strings[regs->vector]);

    if (regs->vector == 8 || regs->vector == 10 || regs->vector == 11 || regs->vector == 12
        || regs->vector == 13 || regs->vector == 14 || regs->vector == 17 || regs->vector == 30)
        log_crit("Interrupt", "[ec 0x%lX]:\n\r", regs->error_code);

    log_crit("INT", "rdi: 0x%p   rsi: 0x%p   rbp: 0x%p   rsp: 0x%p\
\nrbx: 0x%p   rdx: 0x%p   rcx: 0x%p   rax: 0x%p",
        regs->rdi, regs->rsi, regs->rbp, regs->rsp,
        regs->rbx, regs->rdx, regs->rcx, regs->rax);

    log_crit("INT", "r8:  0x%p   r9:  0x%p   r10: 0x%p   r11: 0x%p\
\nr12: 0x%p   r13: 0x%p   r14: 0x%p   r15: 0x%p",
        regs->r8, regs->r9, regs->r10, regs->r11,
        regs->r12, regs->r13, regs->r14, regs->r15);

    log_crit("INT", "cr0: 0x%p   cr2: 0x%p   cr3: 0x%p   cr4: 0x%p",
        regs->cr0, regs->cr2, regs->cr3, regs->cr4);

    log_crit("INT", "EFLAGS: 0x%b\n\n", regs->eflags);
}

/**
 * @brief Handles a kernel panic by dumping register information and logging an error message.
 *
 * @param regs Pointer to the register information (can be NULL).
 * @param quiet Flag indicating whether to display a message or not.
 * @param format Format string for the error message.
 * @param ... Additional arguments for the format string.
 */
void x86_Panic(INT_REG_INFO *regs, uint8_t quiet, const char *format, ...)
{
    (void)quiet;

    if (regs) {
        x86_DumpRegs(regs);
    }

    va_list args;
    va_start(args, format);
    log_err("Kernel Panic", format, args);
    va_end(args);

    __asm__ ("cli\n hlt");
}

/**
 * @brief Handles an unhandled interrupt by logging a warning message.
 *
 * @param regs Pointer to the register information.
 */
void empty_handler(INT_REG_INFO *regs)
{
    int intNumber = regs->vector;
    log_warn("Interrupt", "Unhandled interrupt occured: %x \n", intNumber);
    __asm__ ("hlt");
}

/**
 * @brief Sets the descriptor for an interrupt vector in the Interrupt Descriptor Table (IDT).
 *
 * @param vector The interrupt vector number.
 * @param isr The address of the interrupt service routine.
 * @param flags The flags for the interrupt descriptor.
 */
void x86_IDT_SetDescriptor(uint8_t vector, uintptr_t isr, uint8_t flags) {
    g_IDT_Descriptor *descriptor = &idt[vector];

    descriptor->offset_low = isr & 0xFFFF;
    descriptor->selector = 0x8; // 8: kernel code selector offset (gdt)
    descriptor->ist = 0;
    descriptor->type_attributes = flags;
    descriptor->offset_mid = (isr >> 16) & 0xFFFF;
    descriptor->offset_high = (isr >> 32) & 0xFFFFFFFF;
    descriptor->reserved = 0;
}

/**
 * @brief Initializes the Interrupt Descriptor Table (IDT).
 *
 * This function initializes the Interrupt Descriptor Table (IDT) by setting the
 * descriptor for each interrupt vector. It also sets the default exception handler
 * for each vector.
 */
void x64_IDT_Init(void)
{
    for (size_t vector = 0; vector < 256; vector++) {
        x86_IDT_SetDescriptor(vector, isr_stub_table[vector], 0x8E);
    }
    for (size_t vector = 0; vector < 32; vector++) {
        g_IDT_handlers[vector] = (uintptr_t)default_exception_handler;
    }
    for (size_t vector = 32; vector < 256; vector++) {
        g_IDT_handlers[vector] = (uintptr_t)empty_handler;
    }

    x86_IDT_Load();
}

/**
 * @brief Registers an interrupt vector with the IDT.
 *
 * This function registers an interrupt vector with the IDT by setting the
 * handler function for the vector.
 *
 * @param vector The interrupt vector number.
 * @param handler The address of the interrupt handler function.
 */
void x86_RegisterVector(size_t vector, uintptr_t handler)
{
    if (g_IDT_handlers[vector] != (uintptr_t)empty_handler || !handler) {
        // panic
        log_crit("Interrupt", "Failed to register vector %lu at 0x%p\n", vector, handler);
        __asm__ ("hlt");
    }
    g_IDT_handlers[vector] = handler;
}

/**
 * @brief Unregisters an interrupt vector with the IDT.
 *
 * This function unregisters an interrupt vector with the IDT by setting the
 * handler function for the vector to the empty handler.
 *
 * @param vector The interrupt vector number.
 */
void x86_ResetVector(size_t vector)
{
    if (g_IDT_handlers[vector] == (uintptr_t)empty_handler || vector < 32) {
        // panic
        log_crit("Interrupt", "Failed to erase vector %lu\n", vector);
        __asm__ ("hlt");
    }
    g_IDT_handlers[vector] = (uintptr_t)empty_handler;
}

/**
 * @brief Loads the Interrupt Descriptor Table (IDT) into the CPU.
 */
inline void x86_IDT_Load(void)
{
    idtr.offset = (uintptr_t)idt;
    // max descriptors - 1
    idtr.size = (uint16_t)(sizeof(idt) - 1);

    __asm__ volatile (
        "lidt %0"
        : : "m"(idtr) : "memory"
    );
}