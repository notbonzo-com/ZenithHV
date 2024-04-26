#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <x86/page.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uintptr_t pml4_address;
} pageCTX;

extern pageCTX kernelPmc;

#define PTE_BIT_PRESENT (1ul << 0ul)
#define PTE_BIT_READ_WRITE (1ul << 1ul)
#define PTE_BIT_ACCESS_ALL (1ul << 2ul)
#define PTE_BIT_WRITE_THROUGH_CACHING (1ul << 3ul)
#define PTE_BIT_DISABLE_CACHING (1ul << 4ul)
#define PTE_BIT_PDE_OR_PTE_ACCESSED (1ul << 5ul)
#define PTE_BIT_DIRTY (1ul << 6ul)
#define PTE_BIT_PAT_SUPPORTED (1ul << 7ul)
#define PTE_BIT_GLOBAL (1ul << 8ul)
#define PTE_BIT_EXECUTE_DISABLE (1ul << 63ul)

void initVmm(void);
void mapPage(pageCTX *pmc, uintptr_t va, uintptr_t pa, uint64_t flags);
bool unmapPage(pageCTX *pmc, uintptr_t va, bool free_pa);
uintptr_t virt2phys(pageCTX *pmc, uintptr_t virt);

void setCFX(const pageCTX *pmc);

#ifdef __cplusplus
}
#endif