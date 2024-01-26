/**
 * @file int.h
 * @brief Header file containing declarations for interrupt handling and initialization.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Structure representing the register information during an interrupt.
 */
typedef struct __attribute__((packed)) {
    uint64_t cr4;           /**< CR4 register value */
    uint64_t cr3;           /**< CR3 register value */
    uint64_t cr2;           /**< CR2 register value */
    uint64_t cr0;           /**< CR0 register value */
    uint64_t eflags;        /**< EFLAGS register value */
    uint64_t r15;           /**< R15 register value */
    uint64_t r14;           /**< R14 register value */
    uint64_t r13;           /**< R13 register value */
    uint64_t r12;           /**< R12 register value */
    uint64_t r11;           /**< R11 register value */
    uint64_t r10;           /**< R10 register value */
    uint64_t r9;            /**< R9 register value */
    uint64_t r8;            /**< R8 register value */
    uint64_t rax;           /**< RAX register value */
    uint64_t rcx;           /**< RCX register value */
    uint64_t rdx;           /**< RDX register value */
    uint64_t rbx;           /**< RBX register value */
    uint64_t rsp;           /**< RSP register value */
    uint64_t rbp;           /**< RBP register value */
    uint64_t rsi;           /**< RSI register value */
    uint64_t rdi;           /**< RDI register value */
    uint64_t vector;        /**< Interrupt vector number */
    uint64_t error_code;    /**< Error code associated with the interrupt */
} INT_REG_INFO;

/**
 * @brief Dumps the register information during an interrupt.
 * @param regs Pointer to the INT_REG_INFO structure containing the register information.
 */
void x86_DumpRegs(INT_REG_INFO *regs);

/**
 * @brief Default exception handler.
 * @param regs Pointer to the INT_REG_INFO structure containing the register information.
 */
void default_exception_handler(INT_REG_INFO *regs);

/**
 * @brief Structure representing the Interrupt Descriptor Table (IDT) descriptor.
 */
typedef struct __attribute__((packed)) {
    uint16_t offset_low;        /**< Lower 16 bits of the interrupt handler offset */
    uint16_t selector;          /**< Code segment selector */
    uint8_t ist;                /**< Interrupt Stack Table (IST) index */
    uint8_t type_attributes;    /**< Type and attributes of the interrupt gate */
    uint16_t offset_mid;        /**< Middle 16 bits of the interrupt handler offset */
    uint32_t offset_high;       /**< Upper 32 bits of the interrupt handler offset */
    uint32_t reserved;          /**< Reserved field */
} g_IDT_Descriptor;

/**
 * @brief Registers an interrupt vector with a handler function.
 * @param vector The interrupt vector number.
 * @param handler The address of the interrupt handler function.
 */
void x86_RegisterVector(size_t vector, uintptr_t handler);

/**
 * @brief Resets an interrupt vector to its default state.
 * @param vector The interrupt vector number.
 */
void x86_ResetVector(size_t vector);

/**
 * @brief Sets the descriptor for an interrupt vector in the IDT.
 * @param vector The interrupt vector number.
 * @param isr The address of the interrupt service routine (ISR).
 * @param flags The flags for the interrupt gate.
 */
void x86_IDT_SetDescriptor(uint8_t vector, uintptr_t isr, uint8_t flags);

/**
 * @brief Initializes the Interrupt Descriptor Table (IDT).
 */
void x64_IDT_Init(void);

/**
 * @brief Loads the Interrupt Descriptor Table (IDT) into the processor.
 */
void x86_IDT_Load(void);

/**
 * @brief Generates a panic message and halts the system.
 * @param regs Pointer to the INT_REG_INFO structure containing the register information.
 * @param quiet Flag indicating whether to print the panic message or not.
 * @param format The format string for the panic message.
 * @param ... Additional arguments for the format string.
 */
void x86_Panic(INT_REG_INFO *regs, uint8_t quiet, const char *format, ...);