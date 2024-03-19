#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uintptr_t pml4Address;
} PageMapCtx;
extern PageMapCtx kernelPmc;

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

void initVm(void);
void vmMapSp(PageMapCtx *pmc, uintptr_t va, uintptr_t pa, uint64_t flags);
void vmUnmapSp(PageMapCtx *pmc, uintptr_t va, bool freePa);

void vmSetCFX(const PageMapCtx *pmc);
