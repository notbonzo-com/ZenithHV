#include <stdint.h>
#include <sys/idt.hh>

#if UINT32_MAX == UINTPTR_MAX
#define STACK_CHK_GUARD 0xe2dee396
#else
#define STACK_CHK_GUARD 0x595e9fbd94fda766
#endif

extern "C" {
	uintptr_t __stack_chk_guard = STACK_CHK_GUARD;
	[[noreturn]] void __stack_chk_fail()
	{
		intr::kpanic(nullptr, "stack smashing detected");
		__builtin_unreachable();
	}
}