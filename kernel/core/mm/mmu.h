//
// Created by notbonzo on 2/9/25.
//

#ifndef MMU_H
#define MMU_H

#include <limine.h>
#include <stdint.h>

extern struct limine_kernel_address_response *kernel_address;

struct hypervisor_page_map {
    uintptr_t pml4_address;
    uintptr_t cr3_paddr;
};

extern struct hypervisor_page_map hypervisor_page_map;

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

void init_mmu( );
void map( uintptr_t vaddr, uintptr_t paddr, uint64_t flags );
bool unmap( uintptr_t va, bool free_pa );

#endif //MMU_H
