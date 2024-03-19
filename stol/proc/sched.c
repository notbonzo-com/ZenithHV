#include <proc/sched.h>
#include <mm/vm.h>
#include <mm/page.h>
#include <mm/kalloc.h>
#include <x86/apic.h>
#include <x86/int.h>
#include <x86/cpu.h>
#include <x86/io.h>
#include <x86/pit.h>

#include <debug.h>
#include <mem.h>
#include <binary.h>

#include <proc/queue.h>

VECTOR_TMPL_TYPE(thread_t_ptr)
VECTOR_TMPL_TYPE(task_ptr)
VECTOR_TMPL_TYPE(void_ptr)

vector_task_ptr_t proc_list = VECTOR_INIT(task_ptr);

queue_t thread_queue = QUEUE_INIT_FAST();

static k_lock scheduler_lock;

struct task *kernel_task = NULL;

void init_scheduling(void)
{
    registerVector(INT_VEC_SCHEDULER, (uintptr_t)scheduler_preempt);
    kernel_task = scheduler_add_task(NULL, &kernelPmc);
}

struct task *scheduler_add_task(struct task *parent_proc, PageMapCtx *pmc)
{
    acquire_lock(&scheduler_lock);
    struct task *new_proc = kmalloc(sizeof(struct task));

    VECTOR_REINIT(new_proc->threads, thread_t_ptr);

    if (parent_proc == NULL) {
        new_proc->parent = NULL;
        new_proc->pmc = pmc;
    }

    release_lock(&scheduler_lock);
    return new_proc;
}

thread_t *scheduler_add_kernel_thread(void *entry)
{
    acquire_lock(&scheduler_lock);

    thread_t *new_thread = kcalloc(1, sizeof(thread_t));

    VECTOR_REINIT(new_thread->stacks, void_ptr);

    void *stack_phys = pgmClaimPage(100);
    new_thread->stacks.push_back(&new_thread->stacks, stack_phys);
    void *stack = stack_phys + 100 * PAGE_SIZE + hhdm->offset;

    new_thread->context.cs = 0x8;
    new_thread->context.ds = new_thread->context.es = new_thread->context.ss = 0x10;
    new_thread->context.rflags = 0x202; // interrupts enabled
    new_thread->context.rip = (uintptr_t)entry;
    new_thread->context.rdi = 0; // pass args?
    new_thread->context.rsp = (uint64_t)stack;
    new_thread->context.cr3 = (uint64_t)kernel_task->pmc->pml4Address - hhdm->offset;
    new_thread->gs_base = new_thread;

    new_thread->owner = kernel_task;

    log_debug("Sched", "ktask->tid = %lu (%p)\n", kernel_task->threads.push_back(&kernel_task->threads, new_thread), new_thread);

    queue_enqueue(&thread_queue, (void *)new_thread);

    release_lock(&scheduler_lock);

    return new_thread;
}

void scheduler_yield(void)
{

    cpu_local_t *this_cpu = get_this_cpu();

    lapic_timer_halt();

    lapic_send_ipi(this_cpu->lapic_id, INT_VEC_SCHEDULER, ICR_DEST_SELF);

    enable_interrupts();
    for (;;) __asm__ ("hlt");
}

__attribute__((noreturn)) void wait_for_scheduling(void)
{
    lapic_timer_oneshot_us(INT_VEC_SCHEDULER, 20 * 1000);
    enable_interrupts();
    for (;;) __asm__ ("hlt");
    __builtin_unreachable();
}

void scheduler_preempt(intRegInfo_t *regs)
{
    acquire_lock(&scheduler_lock);

    cpu_local_t *this_cpu = get_this_cpu();
    thread_t *this_thread = get_current_thread();

    if (this_thread->killed) {
        size_t index = this_thread->owner->threads.find(&this_thread->owner->threads, this_thread);
        if (index == VECTOR_NOT_FOUND) {
            panic(NULL, 0, "Trying to free dead or non existing thread (%p)\n", this_thread);
        }
        this_thread->owner->threads.remove(&this_thread->owner->threads, index);

        for (size_t i = 0; i < this_thread->stacks.size; i++) {
            pgmFreePage((void *)((uintptr_t)this_thread->stacks.data[i] - 100 * PAGE_SIZE - hhdm->offset), 100);
        }

        this_thread->stacks.reset(&this_thread->stacks);

        kfree(this_thread);

        log_debug("Shed", "freed thread (%p) successfully\n", this_thread);

        goto killed;
    }

    memcpy(&this_thread->context, regs, sizeof(intRegInfo_t));
    if (this_thread != this_cpu->idle_thread) {
        queue_enqueue(&thread_queue, (void *)this_thread);
    }

killed:
    thread_t *next_thread = (thread_t *)queue_dequeue(&thread_queue);
    if (!next_thread) {
        memcpy(regs, &this_cpu->idle_thread->context, sizeof(intRegInfo_t));

        write_gs_base(this_cpu->idle_thread->gs_base);
        write_kernel_gs_base(this_cpu->idle_thread->gs_base);
        vmSetCFX(this_cpu->idle_thread->owner->pmc);

        lapic_timer_oneshot_us(INT_VEC_SCHEDULER, 20 * 1000);
        lapic_send_eoi_signal();


        release_lock(&scheduler_lock);
        return;
    }

    memcpy(regs, &next_thread->context, sizeof(intRegInfo_t));

    next_thread->cpu = this_cpu;

    write_gs_base(next_thread->gs_base);
    write_kernel_gs_base(next_thread->gs_base);
    vmSetCFX(next_thread->owner->pmc);
    this_cpu->tss.rsp[0] = (uint64_t)next_thread->kernel_stack;

    lapic_timer_oneshot_us(INT_VEC_SCHEDULER, 5 * 1000);
    lapic_send_eoi_signal();

    release_lock(&scheduler_lock);
}

void __attribute__((noreturn)) scheduler_kernel_thread_exit(void)
{
    disable_interrupts();

    struct thread_t *this_thread = get_current_thread();
    this_thread->killed = true;

    scheduler_yield();

    __builtin_unreachable();
}