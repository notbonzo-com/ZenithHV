#include <x86/io.h>
#include <stdint.h>

void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1"
        : : "a" (value), "d" (port) : "memory");
}
uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile ("inb %1, %0"
        : "=a" (value) : "d" (port) : "memory");
    return value;
}
void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile ("outw %0, %1"
        : : "a" (value), "d" (port) : "memory");
}
uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile ("inw %1, %0"
        : "=a" (value) : "d" (port) : "memory");
    return value;
}
void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile ("outl %0, %1"
        : : "a" (value), "d" (port) : "memory");
}
uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile ("inl %1, %0"
        : "=a" (value) : "d" (port) : "memory");
    return value;
}

void halt()
{
    __asm__ volatile ("hlt");
}

void ioWait()
{
    outb(0x80, 0);
}

uint64_t cpuGetMSR(uint32_t msr)
{
    uint32_t hi;
    uint32_t lo;
    asm volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(msr));

    return ((uint64_t)hi << 32) | lo;
}
void cpuSetMSR(uint32_t msr, uint32_t hi, uint32_t low)
{
    asm volatile("wrmsr" : : "c"(msr), "a"(low), "d"(hi));
}