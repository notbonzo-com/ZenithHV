/**
 * @file int.h
 * @brief Header file containing declarations for interrupt handling and initialization.
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define INT_VEC_PIT 32
#define INT_VEC_PS2 33

#define INT_VEC_SCHEDULER 100
#define INT_VEC_LAPIC_TIMER 101

#define INT_VEC_LAPIC_IPI 200

#define INT_VEC_SPURIOUS 254
#define INT_VEC_GENERAL_PURPOSE 255



/**
 * @brief Structure representing the register information during an interrupt.
 */
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
} intRegInfo_t;

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
} idtDesc_t;

/**
 * @brief Dumps the register information during an interrupt.
 * @param regs Pointer to the INT_REG_INFO structure containing the register information.
 */
void dumpRegs(intRegInfo_t *regs);
/**
 * @brief Default exception handler.
 * @param regs Pointer to the INT_REG_INFO structure containing the register information.
 */
void defaultExceptionHandler(intRegInfo_t *regs);
/**
 * @brief Sets the descriptor for an interrupt vector in the IDT.
 * @param vector The interrupt vector number.
 * @param isr The address of the interrupt service routine (ISR).
 * @param flags The flags for the interrupt gate.
 */
void setVector(uint8_t vector, uintptr_t isr, uint8_t flags);
/**
 * @brief Registers an interrupt vector with a handler function.
 * @param vector The interrupt vector number.
 * @param handler The address of the interrupt handler function.
 */
bool registerVector(size_t vector, uintptr_t handler);
/**
 * @brief Resets an interrupt vector to its default state.
 * @param vector The interrupt vector number.
 */
bool resetVector(size_t vector);

/**
 * @brief Initializes the Interrupt Descriptor Table (IDT).
 */
int initIdt(void);
/**
 * @brief Loads the Interrupt Descriptor Table (IDT) into the processor.
 */
void loadIdt(void);
/**
 * @brief Generates a panic message and halts the system.
 * @param regs Pointer to the INT_REG_INFO structure containing the register information.
 * @param quiet Flag indicating whether to print the panic message or not.
 * @param format The format string for the panic message.
 * @param ... Additional arguments for the format string.
 */
void panic(intRegInfo_t *regs, uint8_t quiet, const char *format, ...);
/**
 * @brief Sends an End Of Interrupt (EOI) signal to the Programmable Interrupt Controller (PIC).
 */
void sendEoi(void);