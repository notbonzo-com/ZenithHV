#include <limine.h>
#include <stdint.h>
#include <kprintf>

#include <kmalloc>
#include <io>

#include <sys/gdt.hpp>
#include <sys/idt.hpp>
#include <sys/mm/pmm.hpp>
#include <sys/mm/mmu.hpp>
#include <sys/mm/kheap.hpp>
#include <sys/apci.hpp>
#include <sys/apic.hpp>

extern "C" {
__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

extern void (*start_ctors)();
extern void (*end_ctors)();
extern void (*start_dtors)();
extern void (*end_dtors)();
void *__dso_handle;

void __cxa_finalize(void *f) {(void)f;};
int __cxa_atexit(void (*destructor) (void *), void *arg, void *dso) {
    (void)destructor; (void)arg; (void)dso; return 0;
}
extern void __cxa_pure_virtual(void) {};
extern "C" uint8_t kmain(void);
void __shutdown(void);
}

namespace __cxxabiv1 {
    __extension__ typedef int __guard __attribute__((mode(__DI__)));

    extern "C" int __cxa_guard_acquire (__guard *g) {
        return !*(char *)(g);
    }

    extern "C" void __cxa_guard_release (__guard *g) {
        *(char *)g = 1;
    }

    extern "C" void __cxa_guard_abort (__guard *) {
    }

}

extern "C" [[noreturn]] void _start(void)
{
    io::cli();

    debugf("Initilizing the Global Descriptor Table");
    gdt::init();
    debugf("Initilizing the Task State Segment");
    tss::init();
    debugf("Initilizing the Interrupt Descriptor Table");
    intr::init();
    debugf("Initilizing the Physical Memory Manager");
    pmm::init();
    debugf("Initilizing the Virtual Memory Manager");
    mmu::init();
    debugf("Initilisng the Kernel Heap");
    kheap::init((0xFFul * 1024ul * 1024ul * 4096ul) / PAGE_SIZE);
    debugf("Initilizing the APCI tables");
    apci::parse();
    debugf("Initilizing the IOAPIC");
    ioapic::init();
    debugf("Initilizing the LAPIC");
    lapic::init();


    debugf("Calling KMain");
    uint8_t returnCode = kmain();
    if (returnCode != 0)
    {
        kprintf(" -> Kernel returned with exit code %d\n", returnCode);
        intr::kpanic(NULL, "Kernel Main Returned Non-Zero!");
    }

    debugf("Attempting to shut down...");
    __shutdown();
    debugf("Failed to shut down! Halting");
    asm volatile ("cli"); for(;;)asm volatile ("hlt");
}

void __shutdown(void)
{
    kprintf(" -> QEMU Shutdown\n");
    io::out<uint16_t>(0x604, 0x2000); // QEMU
    for (int i = 0; i < 10000; i++)
        asm volatile ("pause");
    /* Explanation for the loop: QEMU Is restarted and doesnt interrupt everything to shut down, instad shuts down on the next timer interrupt.
    (even if it isnt setup, it keeps in running internally) */
    kprintf(" -> VirtualBox Shutdown\n");
    io::out<uint16_t>(0x4004, 0x3400);
    return;
}