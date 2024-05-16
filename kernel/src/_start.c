#include <stdint.h>

__attribute__((noreturn)) void _start(void)
{
	asm volatile ("cli");


	for(;;) asm volatile ("hlt");
}