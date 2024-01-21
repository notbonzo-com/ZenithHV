#pragma once
#include <drivers/tables/gdt.h>
// #include <drivers/tables/int.h>
//#include <drivers/tables/isr.h>

#include <drivers/debug/e9.h>
#define forever(a) forever_start: a ; goto forever_start

void HAL_Init(void)
{
    x64_GDT_Initilise();
    // x86_64_IDT_Initialize();

    return;
}