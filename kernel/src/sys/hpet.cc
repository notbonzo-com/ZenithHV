#include "smp/smp.hh"
#include <atomic>
#include <sys/hpet.hh>
#include <sys/acpi.hh>
#include <sys/mm/pmm.hh>
#include <sys/apic.hh>
#include <io>

namespace hpet {

    hpetregs_t *regs;
    static std::atomic<bool> initialized = false;

    int init()
    {
        if (initialized.load()) {
            return 1;
        }

        header *hpet = reinterpret_cast<header *>(acpi::get_sdt("HPET"));
        if (hpet == nullptr) {
            return -1;
        }
        regs = reinterpret_cast<hpetregs_t *>( (hpet->addr) + pmm::hhdm->offset );
        regs->counter_val = 0;
        regs->general_config = (1 << 0);

        initialized.store(true);
        return 0;
    }

    void usleep(uint64_t us)
    {
        uint64_t clock_period = regs->capabilities >> 32;
        uint64_t ticks = us * 1'000'000'000 / clock_period;
        volatile size_t target_val = regs->counter_val + ticks;
        while (regs->counter_val < target_val) io::pause(); // todo schedule?
    }

    void set_periodic(uint64_t us, uint8_t vector) {
        uint64_t frequency_hz = (regs->capabilities & 0xFFFFFFFF);
        uint64_t ticks = us * (frequency_hz / 1'000'000);

        regs->general_config = (1 << 2) | (1 << 0);
        regs->counter_val = 0;
        regs->int_status = 0;

        volatile uint64_t* comparator = reinterpret_cast<volatile uint64_t*>(
            reinterpret_cast<uintptr_t>(regs) + 0x100);
        *comparator = ticks;

        kprintf("Redirecting irq0 to vector %u on core %u\n", vector, smp::current()->lapic_id);
        ioapic::redirect(0, vector, smp::current()->lapic_id);
    }

    void stop() {
        regs->general_config &= ~(1 << 0);
    }

}