#include <x86/apic.h>
#include <x86/io.h>
#include <x86/cpuid.h>

void initApic(void)
{
    outb(PIC1_BASE, 0b00010001);
    outb(PIC2_BASE, 0b00010001);

    outb(PIC1_DATA, PIC_MASTER_OFFSET); // idt ISR offset
    outb(PIC2_DATA, PIC_SLAVE_OFFSET); // idt ISR offset

    outb(PIC1_DATA, 0b00000100); // slave PIC at irq2
    outb(PIC2_DATA, 0b00000010); // slave PIC at ir2 of master

    outb(PIC1_DATA, 0b00000101); // 8086_MODE | MASTER
    outb(PIC2_DATA, 0b00000001); // 8086_MODE

    // Mask all IRQs
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    if (cpuid_isApicSupported())
    {
        // Initialize the APIC
        //TODO
    }
}