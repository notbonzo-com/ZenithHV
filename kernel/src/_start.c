#include <stdint.h>
#include <kprintf.h>

#include <_start/gdt.h>

__attribute__((noreturn)) void _start(void)
{
	debugf("Initilizing pre-kernel environment");
	kprintf(" -> Disabling interrupts\n");
	asm volatile ("cli");
	debugf("Initilizing the Global Descriptor Table");
	gdt_init();

	for(;;) asm volatile ("hlt");
}