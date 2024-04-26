#include <stdint.h>
#include <kprintf.h>

#include <x86/gdt.h>
#include <x86/smp.h>
#include <x86/idt.h>
#include <x86/page.h>

#include <mm/vmm.h>
#include <mm/kheap.h>
#include <limine.h>

#include <std/mem.hpp>
#include <std/iostream.hpp>

#include <fs/ata.h>
#include <kalloc.h>

LIMINE_BASE_REVISION(1)

extern "C" void __cxa_pure_virtual() { debugf( "Pure virtual function called" ); }
constexpr size_t oneGigabyteInPages = (1024 * 1024 * 1024) / PAGE_SIZE;

extern "C" [[noreturn]] void _start(void) {
    debugf("Initilising the Global Descriptor Table");
    initGDT();

    debugf("Initilising the Interrupt Descriptor Table");
    initIDT();

    debugf("Initilising the Page Manager");
    initPgm();

    debugf("Initilising the Virtual Memory Manager");
    initVmm();

    debugf("Initilising the Kernel Heap");
    size_t maxHeapSizePages = (usable_pages / 2 < oneGigabyteInPages) ? (usable_pages / 2) : oneGigabyteInPages; /* Max size 50% of usable pages */
    initKheap(maxHeapSizePages);

    debugf("Initilising ATA PIO");
    ata_init();

    debugf("Kernel is ready");

    uint8_t* buffer = (uint8_t*)kmalloc(512);
    ata_read(buffer, 0x0, 1);

    for (int i = 0; i < 512; i++) {
        kprintf("%x ", buffer[i]);
    }
    kprintf("\n");

    for(;;){asm volatile("hlt");}
    __builtin_unreachable();
}