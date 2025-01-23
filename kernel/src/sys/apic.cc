#include "sys/idt.hh"
#include <sys/acpi.hh>
#include <sys/apic.hh>
#include <kprintf>
#include <vector>
#include <sys/mm/pmm.hh>
#include <sys/mm/mmu.hh>
#include <sys/hpet.hh>
#include <atomic>
#include <smp/smp.hh>
#include <kmalloc>
#include <new>
#include <io>
#include <macro.hh>

namespace lapic
{
    uint64_t get_lapic_address() {
        uint64_t lapic_base = io::read_msr(0x1B) & 0xFFFFF000;
        return lapic_base;
    }

    uint32_t read_reg(uint32_t reg) {
        return *reinterpret_cast<volatile uint32_t*>(smp::current()->lapic_address + static_cast<uintptr_t>(reg));
    }

    void write_reg(uint32_t reg, uint32_t value) {
        *reinterpret_cast<volatile uint32_t*>(smp::current()->lapic_address + static_cast<uintptr_t>(reg)) = value;
    }

    size_t get_current_core_id() {
        return read_reg(0x20) >> 24;
    }

    void timer_stop() {
        write_reg(0x380, 0x0);
        write_reg(0x320, (1 << 16));
    }
    
    void send_ipi(uint32_t lapic_id, uint8_t vector) {
        write_reg(0x310, lapic_id << 24);
        write_reg(0x300, vector | (1 << 14));
    }

    bool init() {
        uintptr_t core_lapic_address = get_lapic_address();
        smp::core_t* core = smp::current();
        core->lapic_address = core_lapic_address;

        kprintf(" -> Mapping LAPIC for core %zu\n", core->id);
        mmu::kernel_pmc.map(
            ALIGN_DOWN((core_lapic_address + pmm::hhdm->offset), PAGE_SIZE),
            ALIGN_DOWN(core_lapic_address, PAGE_SIZE),
            mmu::PTE_BIT_PRESENT | mmu::PTE_BIT_READ_WRITE
        );
        core->lapic_address += pmm::hhdm->offset;
        core->lapic_id = (lapic::read_reg(0x20) >> 24) & 0xFF;

        io::out<uint8_t>(0x21, 0xFF);
        io::io_wait();
        io::out<uint8_t>(0xA1, 0xFF);
        io::io_wait();

        write_reg(0x0F0, 0x1FF);
        write_reg(LAPIC_TASKPRIOR, 0);

        if (hpet::init() == -1) {
            return false;
        }

        timer_stop();

        return true;
    }

    void send_eoi() {
        write_reg(0x0B0, 0);
    }

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

        for (size_t i = 0; i < acpi::ioapics.size(); i++) {            
            mmu::kernel_pmc.map(ALIGN_DOWN(((uintptr_t)acpi::ioapics[i]->io_apic_address + pmm::hhdm->offset), PAGE_SIZE),
                ALIGN_DOWN((uintptr_t)acpi::ioapics[i]->io_apic_address, PAGE_SIZE), mmu::PTE_BIT_PRESENT | mmu::PTE_BIT_READ_WRITE);
        }
        
        kprintf(" -> IOAPIC initialization complete\n");
        return 0;
    }

    void write(struct acpi::IOAPIC* ioapic, uint32_t reg, uint32_t value) {
        *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(ioapic->io_apic_address) + pmm::hhdm->offset) = reg;
        *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(ioapic->io_apic_address) + pmm::hhdm->offset + 0x10) = value;
    }

    uint32_t read(struct acpi::IOAPIC* ioapic, uint32_t reg) {
        *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(ioapic->io_apic_address) + pmm::hhdm->offset) = reg;
        return *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(ioapic->io_apic_address) + pmm::hhdm->offset + 0x10);
    }

    void set_entry(uint32_t irq, uint32_t vector, uint32_t lapic_id, bool unset)
    {
        uint16_t iso_flags = 0;
        for (size_t i = 0; i < acpi::isos.size(); i++) {
            if (acpi::isos[i]->source_irq == irq) {
                irq = acpi::isos[i]->global_system_interrupt;
                iso_flags = acpi::isos[i]->flags;
                break;
            }
        }

        struct acpi::IOAPIC* ioapic = nullptr;
        for (auto& ioapic_ptr : acpi::ioapics) {
            auto lambda = [irq](acpi::IOAPIC* ioapic) {
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
            kprintf(" -> acpi IOAPIC count: %lu\n", acpi::ioapics.size());
            for (size_t i = 0; i < acpi::ioapics.size(); i++) {
                kprintf(" -> IOAPIC[%lu]: Address: %lu, GSI Base: %u\n", i, acpi::ioapics[i]->io_apic_address, acpi::ioapics[i]->global_system_interrupt_base);
            }
            intr::kpanic(nullptr, "No IOAPIC Handles the specified range!");
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