#include <sys/apci.hpp>
#include <sys/apic.hpp>
#include <kprintf>
#include <vector>
#include <util>
#include <sys/mm/pmm.hpp>
#include <sys/mm/mmu.hpp>
#include <atomic>
#include <kmalloc>
#include <new>
#include <io>


namespace lapic
{
uintptr_t lapic_address = 0xFEE00000;


} // namespace lapic

namespace ioapic
{
uint64_t init()
{
	io::out<uint8_t>(0x20, 0b00010001);
	io::out<uint8_t>(0xA0, 0b00010001);
	io::out<uint8_t>(0x21, 0x20);
    io::out<uint8_t>(0xA1, 0x28);

    io::out<uint8_t>(0x21, 0b00000100); 
    io::out<uint8_t>(0xA1, 0b00000010);

    io::out<uint8_t>(0x21, 0b00000101); 
    io::out<uint8_t>(0xA1, 0b00000001); 

    io::out<uint8_t>(0x21, 0xFF);
    io::out<uint8_t>(0xA1, 0xFF);

	for (size_t i = 0; i < apci::ioapics.size(); i++) {            
        mmu::map(&mmu::kernel_pmc, ALIGN_DOWN(((uintptr_t)apci::ioapics[i]->io_apic_address + pmm::hhdm->offset), PAGE_SIZE),
            ALIGN_DOWN((uintptr_t)apci::ioapics[i]->io_apic_address, PAGE_SIZE), mmu::PTE_BIT_PRESENT | mmu::PTE_BIT_READ_WRITE);
    }
    
    kprintf(" -> IOAPIC initialization complete\n");
	return 0;
}

void write(struct apci::IOAPIC* ioapic, uint32_t reg, uint32_t value) {
    *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(ioapic->io_apic_address) + pmm::hhdm->offset) = reg;
    *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(ioapic->io_apic_address) + pmm::hhdm->offset + 0x10) = value;
}

uint32_t read(struct apci::IOAPIC* ioapic, uint32_t reg) {
    *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(ioapic->io_apic_address) + pmm::hhdm->offset) = reg;
    return *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(ioapic->io_apic_address) + pmm::hhdm->offset + 0x10);
}

void set_entry(uint32_t irq, uint32_t vector, uint32_t lapic_id, bool unset)
{
    uint16_t iso_flags = 0;
    for (size_t i = 0; i < apci::isos.size(); i++) {
        if (apci::isos[i]->source_irq == irq) {
            irq = apci::isos[i]->global_system_interrupt;
            iso_flags = apci::isos[i]->flags;
            break;
        }
    }

    struct apci::IOAPIC* ioapic = nullptr;
    for (auto& ioapic_ptr : apci::ioapics) {
        auto lambda = [irq](apci::IOAPIC* ioapic) {
            const auto base = ioapic->global_system_interrupt_base;
            const auto end = base + ((read(ioapic, 0x1) & 0xFF0000) >> 16);
            return base <= irq && irq < end;
        };
        if (lambda(ioapic_ptr)) {
            ioapic = ioapic_ptr;
            break;
        }
    }
    if (!ioapic) {
		kprintf(" -> Offending IRQ: %u\n", irq);
        kprintf(" -> apci IOAPIC count: %lu\n", apci::ioapics.size());
        for (size_t i = 0; i < apci::ioapics.size(); i++) {
            kprintf(" -> IOAPIC[%lu]: Address: %lu, GSI Base: %u\n", i, apci::ioapics[i]->io_apic_address, apci::ioapics[i]->global_system_interrupt_base);
        }
        intr::kpanic(NULL, "No IOAPIC Handles the specified range!");
    }

    uint32_t entry_high = lapic_id << 24;
    uint32_t entry_low = vector;

    if (iso_flags & 0b11) {
        entry_low |= 1ul << 13;
    }
    if (iso_flags & 0b1100) {
        entry_low |= 1ul << 15;
    }

    if (unset) {
        entry_low = 0;
        entry_high = 0;
    }
    uint32_t table_index = (irq - ioapic->global_system_interrupt_base) * 2;
    write(ioapic, 0x10 + table_index, entry_low);
    write(ioapic, 0x10 + table_index + 1, entry_high);
}

void redirect(uint32_t irq, uint32_t vector, uint32_t lapic_id)
{
    set_entry(irq, vector, lapic_id, 0);
}

void remove(uint32_t irq, uint32_t lapic_id)
{
    set_entry(irq, 0, lapic_id, 1);
}

} // namespace ioapic
