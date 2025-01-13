#include <stdint.h>

#include <sys/gdt.hpp>
#include <sys/mm/pmm.hpp>

#include <kmalloc>
#include <io>

#include <sys/idt.hpp>

extern "C" uint8_t kmain(void) {

    return 0;
}