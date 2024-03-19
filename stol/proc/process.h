#pragma once

#include <mm/vm.h>
#include <vector.h>
#include <x86/int.h>
#include <x86/gdt.h>
#include <x86/cpu.h>

#include <stdbool.h>

struct cpu_local_t;
struct thread_t;
struct task;

typedef struct thread_t * thread_t_ptr;
typedef struct task * task_ptr;
typedef void * void_ptr;
VECTOR_DECL_TYPE(thread_t_ptr)
VECTOR_DECL_TYPE(task_ptr)
VECTOR_DECL_TYPE(void_ptr)

enum task_state {
    NONE
};

struct task {
    int pid;
    enum task_state state;
    PageMapCtx *pmc;
    vector_thread_t_ptr_t threads;
    vector_task_ptr_t children;
    struct task *parent;
    tss tss;
};

typedef struct thread_t {
    struct task *owner;
    vector_void_ptr_t stacks;
    void *kernel_stack;
    struct cpu_local_t *cpu;
    intRegInfo_t context;
    struct thread_t *gs_base;
    uintptr_t fs_base;
    bool killed;
} thread_t;