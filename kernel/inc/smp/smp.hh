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

        core_t() = default;

        core_t(uint64_t id, uint32_t lapic_id)
        : id(id), lapic_id(lapic_id) {}
    };

    core_t* current();
    size_t core_count();
    core_t* core_by_id(uintptr_t id);

    extern std::vector<core_t*> global_cpus;
    extern uint64_t cpu_count;
    extern bool initialized;

    void init();
    void boot_other_cores();

} // namespace smp
