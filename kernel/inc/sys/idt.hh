#pragma once

#include <stdint.h>
#include <stddef.h>

namespace intr
{
	typedef struct {
		uint64_t es, ds;
		uint64_t CR4, CR3, CR2, CR0;
		uint64_t R15, R14, R13, R12, R11, R10, R9, R8, RAX, RBX, RCX, RDX, RBP, RSI, RDI;
		uint64_t interrupt, error;
		uint64_t rip, cs, rflags, rsp, ss;                   // pushed automatically by CPU
	} __attribute__((packed)) regs_t;
	
	enum class IDT_FLAGS : uint8_t {
		GATE_TASK        = 0x5,
		GATE_64BIT_INT   = 0xE,
		GATE_64BIT_TRAP  = 0xF,

		RING0            = (0 << 5),
		RING1            = (1 << 5),
		RING2            = (2 << 5),
		RING3            = (3 << 5),

		PRESENT          = 0x80
	};

	typedef struct {
		uint16_t offset_low;
		uint16_t selector;
		uint8_t ist;
		uint8_t attribute;
		uint16_t offset_mid;
		uint32_t offset_high;
		uint32_t zero;
	} __attribute__((packed)) entry_t;

	typedef struct {
		uint16_t limit;
		uint64_t offset;
	} __attribute__((packed)) pointer_t;
}

namespace intr
{
	extern __attribute__((aligned(16))) entry_t entries[256];
	extern "C" uintptr_t handlers[256];
	extern "C" uint64_t stubs[];

	void load();
	void init();
	using handler_t = void (*)(regs_t* regs);

	void kpanic(regs_t *regs, const char* str);

	extern "C" void default_interrupt_handler(regs_t* CPU);
	bool register_handler(size_t vector, handler_t handler);
} // namespace intr