#pragma once
#include <init/gdt.h>
#include <init/int.h>
#include <drivers/mm/pg.h>
#include <drivers/mm/vm.h>
#include <drivers/mm/kmalloc.h>
#include <drivers/visual/graphic.h>
#include <stdio.h>

#include <drivers/debug/e9.h>
#define forever(a) forever_start: a ; goto forever_start

void HAL_Init(void)
{
    x64_GDT_Init();
    x64_IDT_Init(); // IDT + ISR
    x86_64_pgm_init(); // Paging + Structure for MMU
    VISUAL_ClrSrc(); // Paging sometime writtes garbage to the frame buffer

    x86_64_VM_Init(); // Virtual Memory
    x86_64_KHeap_Init(0x1000); // Kernel Heap

    return;
}