#include <stdint.h>

#include <sys/gdt.hpp>
#include <sys/mm/pmm.hpp>

#include <gtest>

#include <kmalloc>
#include <vector>
#include <ramfs>

#include <sys/idt.hpp>
#include <sys/apic.hpp>

struct test
{
	uint8_t value;
};

test test1 = { .value = 0, };

extern "C" uint8_t kmain(void)
{


	return 0;
}