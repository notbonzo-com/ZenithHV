#include <limine.h>
#include <stdint.h>
#include <kprintf>

#include <kmalloc>
#include <io>

#include <sys/gdt.hh>
#include <sys/idt.hh>
#include <sys/mm/pmm.hh>
#include <sys/mm/mmu.hh>
#include <sys/mm/kheap.hh>
#include <sys/acpi.hh>
#include <sys/apic.hh>

#include <smp/smp.hh>

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
    void __shutdown(void);
}
uint8_t vmm_main(void);

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
    debugf("Initilizing the Interrupt Descriptor Table");
    intr::init();
    debugf("Initilizing the Physical Memory Manager");
    pmm::init();
    debugf("Initilizing the Virtual Memory Manager");
    mmu::kernel_pmc.init();
    kprintf(" -> Setting the kernel context into cr3\n");
    mmu::kernel_pmc.switch2();
    kprintf(" -> Flushing the cr3\n");
    mmu::tlb_flush();
    kprintf(" -> VMM initialization complete\n");
    debugf("Initilisng the Kernel Heap");
    kheap::init((0xFFul * 1024ul * 1024ul * 4096ul) / PAGE_SIZE);
    debugf("Parsing the ACPI tables");
    acpi::parse();
    debugf("Initializing the IOAPIC");
    ioapic::init();
    debugf("Booting up other cores");
    smp::boot_other_cores();

    debugf("Calling Hypervisor Entry point!");
    uint8_t returnCode = vmm_main();
    if (returnCode != 0)
    {
        kprintf(" -> Hypervisor returned with exit code %d\n", returnCode);
        intr::kpanic(nullptr, "Hypervisor Returned Non-Zero!");
    }

    debugf("Attempting to shut down...");
    __shutdown();
    asm volatile ("cli"); for(;;)asm volatile ("hlt");
}

void __shutdown(void)
{
    kprintf(" -> QEMU Shutdown\n");
    io::out<uint16_t>(0x604, 0x2000); // QEMU
    kprintf(" -> VirtualBox Shutdown\n");
    io::out<uint16_t>(0x4004, 0x3400);
    return;
}