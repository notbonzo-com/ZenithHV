#include <limine.h>
#include <stdint.h>
#include <kprintf>
#include <intr.hpp>
#include <pmm.hpp>

#include <kmalloc>
#include <io>

#include <_start/gdt.hpp>
#include <_start/idt.hpp>
#include <_start/pmm.hpp>
#include <_start/vmm.hpp>
#include <_start/kheap.hpp>
#include <_start/acpi.hpp>

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
extern "C" uint8_t kmain(void);
extern "C" [[noreturn]] void __preinit(void);
void __shutdown(void);
extern void __cxa_pure_virtual(void) {};
}

extern "C" [[noreturn]] void _start(void)
{
    debugf("Initilising the Global Descriptor Table");
    gdt::init();
    debugf("Initilising the Task State Segment");
    tss::init();
    debugf("Initilising the Interrupt Descriptor Table");
    intr::init();
    debugf("Initilising the Physical Memory Manager");
    pmm::init();
    debugf("Initilising the Virtual Memory Manager");
    vmm::init();
    debugf("Initilisng the Kernel Heap");
    kheap::init((0xFFul * 1024ul * 1024ul * 4096ul) / PAGE_SIZE);
    debugf("Initilising the ACPI tables");
    acpi::parse();

    __preinit();
}

extern "C" [[noreturn]] void __preinit(void)
{
    debugf("Calling the Global Constructors")
    for (void (**ctor)() = &start_ctors; ctor != &end_ctors; ++ctor)
    {
        (*ctor)();
    }
    debugf("Calling KMain");
    uint8_t returnCode = kmain();
    if (returnCode != 0)
    {
        kprintf(" -> Kernel returned with exit code %d\n", returnCode);
        intr::kpanic(NULL, "Kernel Main Returned Non-Zero!");
    }
    kprintf(" -> Kernel returned with exit code %d\n", returnCode);
    debugf("Calling the Global Destructors")
    for (void (**dtor)() = &start_dtors; dtor != &end_dtors; ++dtor)
    {
        (*dtor)();
    }
    debugf("Attempting to shut down...");
    __shutdown();
    debugf("Failed to shut down! Halting");
    asm volatile ("cli"); for(;;)asm volatile ("hlt");
}

bool __get_cpuid(uint32_t op, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    __asm__ __volatile__(
        "cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "0"(op)
    );
    return true;
}

void __shutdown(void)
{
    kprintf(" -> QEMU Shutdown\n");
    outw(0x604, 0x2000); // QEMU
    kprintf(" -> VirtualBox Shutdown\n");
    outw(0x4004, 0x3400);
    return;
}