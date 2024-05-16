#include <stdint.h>
#include <kprintf.h>

#include <_start/gdt.h>
#include <_start/idt.h>

extern __attribute__((noreturn)) void __preinit(void);

__attribute__((noreturn)) void _start(void)
{
	debugf("Initilizing pre-kernel environment");
	kprintf(" -> Disabling interrupts\n");
	asm volatile ("cli");
	debugf("Initilizing the Global Descriptor Table");
	gdt_init();
	debugf("Initilizing the Interrupt Descriptor Table");
	idt_init();


	debugf("Calling __preinit");
	__preinit();
}

