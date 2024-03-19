#include <x86/apic.h>
#include <mem.h>
#include <mm/vm.h>
#include <mm/page.h>
#include <x86/io.h>
#include <debug.h>
#include <stdio.h>
#include <x86/pit.h>
#include <proc/sched.h>
#include <binary.h>

static uint64_t calibration_probe_count, calibration_timer_start, calibration_timer_end;

void init_ioapic(void)
{
    outb(0x20, 0b00010001); // INIT | SEND_ICW4
    outb(0xA0, 0b00010001); // INIT | SEND_ICW4

    // ICW2 data:
    outb(0x21, PIC_MASTER_OFFSET); // idt ISR offset
    outb(0xA1, PIC_SLAVE_OFFSET); // idt ISR offset

    // ICW3 data:
    outb(0x21, 0b00000100); // slave PIC at irq2
    outb(0xA1, 0b00000010); // slave PIC at ir2 of master

    // ICW2 data:
    outb(0x21, 0b00000101); // 8086_MODE | MASTER
    outb(0xA1, 0b00000001); // 8086_MODE

    // mask all interrupts
    outb(0x21, 0xFF);
    outb(0xA1, 0xFF);

    for (size_t i = 0; i < ioapics.size; i++) {
        vmMapSp(&kernelPmc, ALIGN_DOWN(((uintptr_t)ioapics.data[i]->io_apic_address + hhdm->offset), PAGE_SIZE),
            ALIGN_DOWN((uintptr_t)ioapics.data[i]->io_apic_address, PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    }
}

void ioapic_write(struct acpi_ioapic *ioapic, uint32_t reg, uint32_t val)
{
    *(volatile uint32_t *)((uintptr_t)ioapic->io_apic_address + hhdm->offset) = reg;
    *(volatile uint32_t *)((uintptr_t)ioapic->io_apic_address + hhdm->offset + 0x10) = val;
}

uint32_t ioapic_read(struct acpi_ioapic *ioapic, uint32_t reg)
{
    *(volatile uint32_t *)((uintptr_t)ioapic->io_apic_address + hhdm->offset) = reg;
    return *(volatile uint32_t *)((uintptr_t)ioapic->io_apic_address + hhdm->offset + 0x10);
}

void ioapic_set_irq(uint32_t irq, uint32_t vector, uint32_t lapic_id, bool unset)
{
    uint16_t iso_flags = 0;
    for (size_t i = 0; i < isos.size; i++) {
        if (isos.data[i]->source_irq == irq) {
            irq = isos.data[i]->global_system_interrupt;
            iso_flags = isos.data[i]->flags;
            break;
        }
    }

    struct acpi_ioapic *ioapic = NULL;
    for (size_t i = 0; i < ioapics.size; i++) {
        if (ioapics.data[i]->global_system_interrupt_base <= irq
            && irq < ioapics.data[i]->global_system_interrupt_base + ((ioapic_read(ioapics.data[i], 0x1) & 0xFF0000) >> 16)) {
            ioapic = ioapics.data[i];
        }
    }
    if (!ioapic) {
        panic(NULL, 0, "IRQL %u isn't mapped to any IOAPIC!\n", (uint64_t)irq);
    }

    uint32_t entry_high = lapic_id << (56 - 32);
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
    ioapic_write(ioapic, 0x10 + table_index, entry_low);
    ioapic_write(ioapic, 0x10 + table_index + 1, entry_high);
}

void ioapic_redirect_irq(uint32_t irq, uint32_t vector, uint32_t lapic_id)
{
    ioapic_set_irq(irq, vector, lapic_id, 0);
}

void ioapic_remove_irq(uint32_t irq, uint32_t lapic_id)
{
    ioapic_set_irq(irq, 0, lapic_id, 1);
}

uintptr_t lapic_address = 0xFEE00000;

inline uint32_t lapic_read(uint32_t reg)
{
    return *((volatile uint32_t *)(lapic_address + reg));
}

inline void lapic_write(uint32_t reg, uint32_t val)
{
    *((volatile uint32_t *)(lapic_address + reg)) = val;
}

void ipi_handler(intRegInfo_t *regs)
{
    (void)regs;
    disable_interrupts();

    cpu_local_t *this_cpu = get_this_cpu();
    log_debug("APIC", "CPU-ID: %u", this_cpu->id);

    __asm__ ("hlt");
}

size_t lapic_vectors_are_registered = 0;
void init_lapic(void)
{
    if ((read_msr(0x1b) & 0xfffff000) != lapic_address - hhdm->offset) {
        panic(NULL, 0, "LAPIC PA doesn't match\n");
    }

    if (!lapic_vectors_are_registered) {
        lapic_vectors_are_registered = 1;
        registerVector(INT_VEC_LAPIC_TIMER, (uintptr_t)lapic_timer_handler);
        registerVector(INT_VEC_LAPIC_IPI, (uintptr_t)ipi_handler);
        registerVector(INT_VEC_SPURIOUS, (uintptr_t)defaultExceptionHandler);
    }
    lapic_write(LAPIC_SPURIOUS_INT_VEC_REG, lapic_read(LAPIC_SPURIOUS_INT_VEC_REG) | 0x100 | 0xFF);

    calibrate_lapic_timer();
}

inline void lapic_send_eoi_signal(void)
{
    // non zero value may cause #GP
    lapic_write(LAPIC_EOI_REG, 0x00);
}

static void calibrate_lapic_timer_pit_callback(intRegInfo_t *regs)
{
    (void)regs;
    uint32_t lapic_tmr_count = lapic_read(LAPIC_TIMER_CURRENT_COUNT_REG);

	calibration_probe_count++;

	if (calibration_probe_count == 1) {
		calibration_timer_start = lapic_tmr_count;
	}
	else if (calibration_probe_count == LAPIC_TIMER_CALIBRATION_PROBES) {
		calibration_timer_end = lapic_tmr_count;
	}
    lapic_send_eoi_signal();
}


void calibrate_lapic_timer(void)
{
    lapic_write(LAPIC_TIMER_CURRENT_COUNT_REG, 0);

    uint32_t reg_old = lapic_read(LAPIC_TIMER_DIV_CONFIG_REG);
    lapic_write(LAPIC_TIMER_DIV_CONFIG_REG, reg_old | 0b1011);

    reg_old = lapic_read(LAPIC_LVT_TIMER_REG);
    lapic_write(LAPIC_LVT_TIMER_REG, LAPIC_TIMER_LVTTR_MASKED);

    lapic_write(LAPIC_TIMER_INITIAL_COUNT_REG, 0xFFFFFFFF);

    calibration_probe_count = 0;

    pit_rate_init(LAPIC_TIMER_CALIBRATION_FREQ);
    ioapic_redirect_irq(0, INT_VEC_GENERAL_PURPOSE, get_this_cpu()->lapic_id);
    registerVector(INT_VEC_GENERAL_PURPOSE, (uintptr_t)calibrate_lapic_timer_pit_callback);
    enable_interrupts();

    while (calibration_probe_count < LAPIC_TIMER_CALIBRATION_PROBES) {
        __asm__ ("pause");
    }

    disable_interrupts();

    ioapic_remove_irq(0, get_this_cpu()->lapic_id);
    resetVector(INT_VEC_GENERAL_PURPOSE);

    uint64_t timer_delta = calibration_timer_start - calibration_timer_end;

    get_this_cpu()->lapic_clock_frequency = (timer_delta / LAPIC_TIMER_CALIBRATION_PROBES - 1) * LAPIC_TIMER_CALIBRATION_FREQ;
}

void lapic_timer_handler(intRegInfo_t *regs)
{
    (void)regs;
    cpu_local_t *this_cpu = get_this_cpu();
    log_debug("APIC", "oneshot signal at cpu %lu\n", this_cpu->id);
    (void)this_cpu;

    lapic_send_eoi_signal();
}

void lapic_timer_periodic(size_t vector, size_t freq)
{
    cpu_local_t *this_cpu = get_this_cpu();
    uint32_t count_per_tick = this_cpu->lapic_clock_frequency / freq;

    lapic_write(LAPIC_TIMER_DIV_CONFIG_REG, 0b1011);
	lapic_write(LAPIC_TIMER_INITIAL_COUNT_REG, count_per_tick);
	lapic_write(LAPIC_LVT_TIMER_REG, vector | LAPIC_TIMER_LVTTR_PERIODIC);
}

void lapic_timer_halt(void)
{
    lapic_write(LAPIC_LVT_TIMER_REG, 1 << 16);
    lapic_write(LAPIC_TIMER_INITIAL_COUNT_REG, 0);
}

void lapic_timer_oneshot_us(size_t vector, size_t us)
{
    cpu_local_t *this_cpu = get_this_cpu();
    uint32_t ticks_per_us = this_cpu->lapic_clock_frequency / 1000000ul;

    lapic_write(LAPIC_TIMER_DIV_CONFIG_REG, 0b1011);
	lapic_write(LAPIC_TIMER_INITIAL_COUNT_REG, ticks_per_us * us);
	lapic_write(LAPIC_LVT_TIMER_REG, vector | LAPIC_TIMER_LVTTR_ONESHOT);
}

void lapic_timer_oneshot_ms(size_t vector, size_t ms)
{
    cpu_local_t *this_cpu = get_this_cpu();
    uint32_t ticks_per_ms = this_cpu->lapic_clock_frequency / 1000ul / 128ul;

    lapic_write(LAPIC_TIMER_DIV_CONFIG_REG, 0b1010);
	lapic_write(LAPIC_TIMER_INITIAL_COUNT_REG, ticks_per_ms * ms);
	lapic_write(LAPIC_LVT_TIMER_REG, vector | LAPIC_TIMER_LVTTR_ONESHOT);
}

void lapic_send_ipi(uint32_t lapic_id, uint32_t vector, enum LAPIC_ICR_DEST dest)
{
    uint32_t icr_low = lapic_read(LAPIC_ICR_REG_LOW), icr_high;

    while (icr_low & (LAPIC_ICR_PENDING << 12))
        __asm__ ("pause");

    icr_low = lapic_read(LAPIC_ICR_REG_LOW) & 0xFFF32000; // clear everything
    icr_high = lapic_read(LAPIC_ICR_REG_HIGH) & 0x00FFFFFF; // clear del field

    switch (dest)
    {
    case ICR_DEST_ALL:
        lapic_write(LAPIC_ICR_REG_HIGH, icr_high);
        lapic_write(LAPIC_ICR_REG_LOW, icr_low | (dest << 18) | vector);
        break;
    case ICR_DEST_OTHERS:
        lapic_write(LAPIC_ICR_REG_HIGH, icr_high);
        lapic_write(LAPIC_ICR_REG_LOW, icr_low | (dest << 18) | vector);
        break;
    case ICR_DEST_SELF:
        lapic_write(LAPIC_ICR_REG_HIGH, icr_high);
        lapic_write(LAPIC_ICR_REG_LOW, icr_low | (dest << 18) | vector);
        break;
    case ICR_DEST_FIELD:
        lapic_write(LAPIC_ICR_REG_HIGH, icr_high | (lapic_id << 24));
        lapic_write(LAPIC_ICR_REG_LOW, icr_low | (dest << 18) | vector);
        break;
    default:
        log_debug("APIC", "Lapic-Send: Unknown Destination\n");
    }
}