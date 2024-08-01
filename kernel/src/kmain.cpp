#include <stdint.h>

#include <sys/gdt.hpp>
#include <sys/mm/pmm.hpp>

#include <gtest>

#include <kmalloc>
#include <vector>
#include <ramfs>
#include <new>
#include <io>

#include <sys/idt.hpp>
#include <sys/apic.hpp>

#include <memory>
#include <string>


extern "C" uint8_t kmain(void)
{


	return 0;
}