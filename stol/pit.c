#include <x86/pit.h>
#include <x86/io.h>
#include <x86/int.h>
#include <x86/cpu.h>

#include <binary.h>
#include <x86/smp.h>

void pit_rate_init(size_t freq)
{
    uint16_t reload_value = (uint16_t)DIV_ROUNDUP(PIT_OSCILLATOR_FREQUENCY, freq);
    outb(0x43, 0b00110100);
    outb(0x40, (uint8_t)reload_value);
    outb(0x40, (uint8_t)(reload_value >> 8));
}

uint16_t pit_read_current(void)
{
    outb(0x43, 0b00000000);
    uint16_t count = (uint16_t)inb(0x40);
    return count | ((uint16_t)inb(0x40) << 8);
}

volatile size_t pit_ticks = 0;
static void pit_handler(intRegInfo_t *regs)
{
    (void)regs;
    pit_ticks++;
    common_timer_handler();
    lapic_send_eoi_signal();
}

void init_pit(void)
{
    pit_rate_init(PIT_INT_FREQUENCY);

    registerVector(INT_VEC_PIT, (uintptr_t)pit_handler);
    ioapic_redirect_irq(0, INT_VEC_PIT, smp_request.response->bsp_lapic_id);
}