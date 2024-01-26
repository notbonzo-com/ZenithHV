#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef struct {
    uintptr_t pml4_address;
} page_map_ctx; extern page_map_ctx kernel_pmc;

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

void x86_64_VM_Init(void);
void x86_64_VM_MapSP(page_map_ctx *pmc, uintptr_t va, uintptr_t pa, uint64_t flags);
void x86_64_VM_UnmapSP(page_map_ctx *pmc, uintptr_t va, bool free_pa);

void x86_64_VM_SetCTX(const page_map_ctx *pmc);