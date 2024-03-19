#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <stdio.h>
#include <debug.h>
#include <mem.h>

#define forever(a) for(;;) { a; }

#include <gdt.h>
#include <int.h>
#include <x86/io.h>
#include <mm/page.h>
#include <mm/vm.h>
#include <mm/kheap.h>

#include <x86/cpuid.h>
#include <x86/acpi.h>
#include <x86/apic.h>

#include <mm/kalloc.h>
#include <vga/init.h>
#include <fs/ata.h>
#include <fs/mbr.h>
/**
 * @brief The entry point of the kernel
 * 
 */

void _start(void) {
    asm volatile ("cli");
    if (initGdt() == -1) {
        log_fail("GDT", "Failed to initialize GDT"); asm("cli"); halt();
    }
    log_pass("GDT", "Initialized GDT");
    if (initIdt() == -1) {
        log_fail("IDT", "Failed to initialize IDT"); asm("cli"); halt();
    }
    log_pass("IDT", "Initialized IDT");
    
    initPgm();
    log_pass("MM", "Initilized the page manager");
    initVm();
    log_pass("MM", "Initilized the virtual memory manager");
    initKernelHeap((0xFFul * 1024ul * 1024ul * 4096ul) / PAGE_SIZE);
    log_pass("MM", "Initilized the kernel heap");
    VISUAL_Init();
    log_pass("VGA", "Initialized the VGA driver");
    initAcpi();
    log_pass("ACPI", "Parsed ACPI tables");
    initApic();
    log_pass("APIC", "Initialized APIC");
    
    log_pass("KERNEL", "Initialized the kernel");

    initMbr();


    forever(
        asm ("hlt")
    );
}