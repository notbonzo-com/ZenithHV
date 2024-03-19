#pragma once

#include <x86/int.h>
#include <x86/cpu.h>
#include <mm/vm.h>
#include <proc/process.h>
#include <vector.h>

extern struct task *kernel_task;

#define SCHEDULER_MAX_THREADS

void init_scheduling(void);
struct task *scheduler_add_task(struct task *parent_proc, PageMapCtx *pmc);
thread_t *scheduler_add_kernel_thread(void *entry);
void scheduler_preempt(intRegInfo_t *regs);
void __attribute__((noreturn)) wait_for_scheduling(void);
void __attribute__((noreturn)) scheduler_kernel_thread_exit(void);

static inline thread_t *get_current_thread(void) {
    return (thread_t *)read_kernel_gs_base();
}

static inline cpu_local_t *get_this_cpu(void)
{
    if (interrupts_enabled())
        panic(NULL, 0, "It's illegal to get_this_cpu() while IF set\n");
    thread_t *current_thread = get_current_thread();
    return current_thread->cpu;
}