#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <limine.h>

namespace mmu
{

extern struct limine_kernel_address_response *kernel_address;


struct page_map_ctx {
    uintptr_t pml4_address;

    void init();
    void switch2();

    void map(uintptr_t va, uintptr_t pa, uint64_t flags);
    bool unmap(uintptr_t va, bool free_pa);

    static uint64_t *pml4_to_pt(uint64_t *pml4, uint64_t va, bool force);
    static uint64_t *get_below_pml(uint64_t *pml_pointer, uint64_t index, bool force);
};

extern page_map_ctx kernel_pmc;

constexpr uint64_t PTE_BIT_PRESENT = 1ul << 0;
constexpr uint64_t PTE_BIT_READ_WRITE = 1ul << 1;
constexpr uint64_t PTE_BIT_ACCESS_ALL = 1ul << 2;
constexpr uint64_t PTE_BIT_WRITE_THROUGH_CACHING = 1ul << 3;
constexpr uint64_t PTE_BIT_DISABLE_CACHING = 1ul << 4;
constexpr uint64_t PTE_BIT_PDE_OR_PTE_ACCESSED = 1ul << 5;
constexpr uint64_t PTE_BIT_DIRTY = 1ul << 6;
constexpr uint64_t PTE_BIT_PAT_SUPPORTED = 1ul << 7;
constexpr uint64_t PTE_BIT_GLOBAL = 1ul << 8;
constexpr uint64_t PTE_BIT_EXECUTE_DISABLE = 1ul << 63;

uintptr_t virt2phys(page_map_ctx *pmc, uintptr_t virt);

static inline void tlb_flush()
{
    __asm__ volatile (
        "movq %%cr3, %%rax\n\
        movq %%rax, %%cr3\n"
        : : : "rax"
   );
}

} // namespace vmm
