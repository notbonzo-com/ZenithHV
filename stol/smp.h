#pragma once

#include <limine.h>
#include <x86/cpu.h>
#include <proc/sched.h>

#include <stddef.h>
#include <stdint.h>

extern struct limine_smp_request smp_request;

extern struct limine_smp_response *smp_response;
extern cpu_local_t *global_cpus;
extern uint64_t smp_cpu_count;

void boot_other_cores(void);