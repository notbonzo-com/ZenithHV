#pragma once

#include <limine.h>
#include <sys/apic.hh>

#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <atomic>
#include <atomic>

namespace smp {

    struct core_t {
        uintptr_t id;
        uint64_t lapic_id;
        uintptr_t lapic_address;
        uint32_t lapic_ticks;
        uint64_t calibration_timer_start;
        uint64_t calibration_timer_end;
        volatile uint32_t calibration_probe_count;
        uint64_t lapic_timer_frequency;

        core_t() = default;

        core_t(uint64_t id, uint32_t lapic_id)
        : id(id), lapic_id(lapic_id), lapic_ticks(0), 
          calibration_timer_start(0), calibration_timer_end(0), 
          calibration_probe_count(0), lapic_timer_frequency(0) {}
    };

    core_t* current();
    core_t* get(size_t core_id);

    extern std::vector<core_t> global_cpus;
    extern uint64_t cpu_count;
    extern bool initialized;

    void init();
    void boot_other_cores();

} // namespace smp
