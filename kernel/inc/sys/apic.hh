#pragma once

#include <stdint.h>
#include <stddef.h>
#include <sys/acpi.hh>
#include <sys/idt.hh>

namespace ioapic
{

    constexpr uint8_t REGSEL = 0x0;
    constexpr uint8_t IOWIN  = 0x10;

    constexpr uint8_t ID     = 0x0;
    constexpr uint8_t VER    = 0x01;
    constexpr uint8_t ARB    = 0x02;
    constexpr uint8_t REDTBL = 0x10;

    uint64_t init();
    void write(acpi::IOAPIC* ioapic, uint32_t reg, uint32_t value);
    uint32_t read(acpi::IOAPIC* ioapic, uint32_t reg);

    void set_entry(uint32_t irq, uint32_t vector, uint32_t lapic_id, bool unset);
    void redirect(uint32_t irq, uint32_t vect, uint32_t lapic_id);
    void remove(uint32_t irq, uint32_t lapic_id);

}

namespace lapic {

    constexpr int PIC_MASTER_OFFSET = 0x20;
    constexpr int PIC_SLAVE_OFFSET = 0x28;

    constexpr int LAPIC_APICID = 0x20;
    constexpr int LAPIC_APICVER = 0x30;
    constexpr int LAPIC_TASKPRIOR = 0x80;

    constexpr int LAPIC_SPURIOUS_INT_VEC_REG = 0x0F0;
    constexpr int LAPIC_EOI_REG = 0x0B0;
    constexpr int LAPIC_LVT_CMCI_REG = 0x2F0;

    constexpr int LAPIC_LVT_TIMER_REG = 0x320;
    constexpr int LAPIC_TIMER_LVTTR_ONESHOT = (0b00 << 17);
    constexpr int LAPIC_TIMER_LVTTR_PERIODIC = (0b01 << 17);
    constexpr int LAPIC_TIMER_LVTTR_MASKED = (0b1 << 16);

    constexpr int LAPIC_LVT_THERMAL_MONITOR_REG = 0x330;
    constexpr int LAPIC_LVT_PERF_COUNTER_REG = 0x340;
    constexpr int LAPIC_LVT_LINT0_REG = 0x350;
    constexpr int LAPIC_LVT_LINT1_REG = 0x360;
    constexpr int LAPIC_LVT_ERROR_REG = 0x370;
    constexpr int LAPIC_LVT_PENDING = (0b1 << 12);

    constexpr int LAPIC_TIMER_INITIAL_COUNT_REG = 0x380;
    constexpr int LAPIC_TIMER_CURRENT_COUNT_REG = 0x390;
    constexpr int LAPIC_TIMER_DIV_CONFIG_REG = 0x3E0;

    extern uintptr_t lapic_address;

    bool init();
    void send_eoi();

    uint32_t read_reg(uint32_t reg);
    void write_reg(uint32_t reg, uint32_t value);

    void send_ipi(uint32_t lapic_id, uint8_t vector);

    void timer_stop();
    void timer_oneshot(uint64_t ms, uint8_t vec);
    void timer_periodic(uint32_t frequency, uint8_t vector);
    size_t get_current_core_id();

}