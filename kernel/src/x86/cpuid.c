#include <x86/cpuid.h>
#include <stdint.h>

void cpuid_getCpuVendor(char str[static 13])
{
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;

    asm volatile ("cpuid"
                  : "=b"(ebx), "=d"(edx), "=c"(ecx) // Output operands
                  : "a"(eax)                        // Input operands
                  : );                              // Clobbered registers

    *(uint32_t *)(str) = ebx;
    *(uint32_t *)(str + 4) = edx;
    *(uint32_t *)(str + 8) = ecx;
    str[12] = '\0';
}

bool cpuid_isApicSupported(void)
{
    uint32_t eax = 1, ebx = 0, ecx = 0, edx = 0;
    asm volatile ("cpuid"
                  : "=b"(ebx), "=d"(edx), "=c"(ecx) // Output operands
                  : "a"(eax)                        // Input operands
                  : );                              // Clobbered registers
    return ecx & (1 << 9);
}