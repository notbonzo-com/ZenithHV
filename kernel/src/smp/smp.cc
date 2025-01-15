#include "kprintf"
#include <limine.h>

#include <stddef.h>
#include <stdint.h>

#include <sys/gdt.hh>
#include <sys/idt.hh>
#include <sys/mm/mmu.hh>
#include <sys/acpi.hh>
#include <sys/apic.hh>

#include <smp/smp.hh>
#include <cpuid.hh>
#include <atomic>
#include <vector>
#include <io>

struct limine_smp_request smp_request = {
    .id = LIMINE_SMP_REQUEST,
    .revision = 0,
    .response = nullptr,
    .flags = 0,
};

namespace smp {

    core_t* current() {
        uintptr_t gs_base;
        asm volatile("mov %%gs:0, %0" : "=r"(gs_base));
        return reinterpret_cast<core_t*>(gs_base);
    }

    core_t* get(size_t core_id) {
        if (core_id >= global_cpus.size()) {
            return nullptr;
        }
        return &global_cpus[core_id];
    }

    bool tscp_supported() {
        uint32_t eax, ebx, ecx, edx;
        if (!__get_cpuid(0x80000000, &eax, &ebx, &ecx, &edx)) {
            return false;
        }
        if (eax < 0x80000001) {
            return false;
        }
        if (!__get_cpuid(0x80000001, &eax, &ebx, &ecx, &edx)) {
            return false;
        }
        return edx & (1 << 27);
    }

    std::vector<core_t> global_cpus;
    uint64_t cpu_count = 0;
    bool initialized = false;

    static struct limine_smp_response* smp_response = nullptr;
    static std::atomic<size_t> startup_checksum = 0;
    static std::klock somelock;

    static void processor_core_entry(struct limine_smp_info* smp_info) {
        gdt::reload();
        intr::init();

        mmu::kernel_pmc.switch2();

        auto current_cpu = reinterpret_cast<core_t*>(smp_info->extra_argument);
        current_cpu->lapic_id = io::read_msr(0x1B) & 0xFFFFF000;

        io::write_gs_base((uintptr_t)current_cpu);
        io::write_kernel_gs_base((uintptr_t)current_cpu);

        if (tscp_supported()) {
            io::write_tsc_aux(current_cpu->id);
        }

        io::write_msr(0x1A0, io::read_msr(0x1A0) & ~(1ul << 22)); // Disable APIC virtualization if enabled (dont forget I did this)

        {
            std::auto_lock al(somelock);
            lapic::init();
        }

        kprintf("  - CPU %lu: LAPIC ID=%u, bus frequency=%lu MHz booted up\n",
            current_cpu->id, current_cpu->lapic_timer_frequency / 1'000'000);
        startup_checksum.store(startup_checksum.load() + 1);

        if (current_cpu->id == smp_response->bsp_lapic_id) {
            return;
        }

        for(;;) io::hlt();
    }

    void boot_other_cores() {
        smp_response = smp_request.response;
        cpu_count = smp_response->cpu_count;

        for (size_t i = 0; i < cpu_count; i++) {
            auto* smp_info = smp_response->cpus[i];

            global_cpus.emplace_back(
                smp_info->processor_id,
                smp_info->lapic_id
            );
            core_t& cpu = global_cpus.back();

            smp_info->extra_argument = reinterpret_cast<uintptr_t>(&cpu);

            if (smp_info->lapic_id == smp_response->bsp_lapic_id) {
                processor_core_entry(smp_info);
                continue;
            }

            smp_info->goto_address = processor_core_entry;
        }

        while (startup_checksum.load() < cpu_count) {
            __asm__ volatile ("pause");
        }

        initialized = true;
    }


}