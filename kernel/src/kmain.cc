#include <stdint.h>

#include <sys/gdt.hh>
#include <sys/mm/pmm.hh>

#include <kmalloc>
#include <io>

#include <sys/idt.hh>

extern "C" uint8_t kmain(void) {

    return 0;
}