#include <sys/hpet.hpp>
#include <sys/apci.hpp>
#include <sys/mm/pmm.hpp>

namespace hpet {

regs_t *regs;


bool init()
{
	header *hpet = reinterpret_cast<header *>(apci::get_sdt("HPET"));
    if (hpet == NULL) {
        return false;
    }
    regs = reinterpret_cast<regs_t *>( (hpet->addr) + pmm::hhdm->offset );
    regs->counter_val = 0;
    regs->general_config = 1;

    return true;
}

void usleep(uint64_t us)
{
	uint32_t clock_period = regs->capabilities >> 32;
    volatile size_t target_val = regs->counter_val + (us * (1000000000 / clock_period));
    while (regs->counter_val < target_val);
}

}