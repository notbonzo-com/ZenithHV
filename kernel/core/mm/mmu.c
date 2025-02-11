//
// Created by notbonzo on 2/9/25.
//

#include <core/mm/mmu.h>
#include <core/mm/pmm.h>
#include <smp/lock.h>
#include <arch/x86_64/intr.h>
#include <string.h>
#ifdef DEBUG
#include <core/printf.h>
#endif

#define ALIGN_UP(x, base) (((x) + (base) - 1) & ~((base) - 1))
#define ALIGN_DOWN(x, base) ((x) & ~((base) - 1))

struct limine_kernel_address_response *kernel_address;

struct hypervisor_page_map hypervisor_page_map;

struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
    .response = nullptr
};

extern char text_start_addr[], text_end_addr[],
        rodata_start_addr[], rodata_end_addr[],
        data_start_addr[], data_end_addr[];

spinlock_t mmu_lock;

static uint64_t *get_below_pml(uint64_t *pml_pointer, uint64_t index, bool force);
static uint64_t *pml4_to_pt(uint64_t *pml4, uint64_t va, bool force);

static void tlb_flush()
{
    __asm__ volatile (
        "movq %%cr3, %%rax\n\
        movq %%rax, %%cr3\n"
        : : : "rax"
   );
}

void load_context( );

void init_mmu( ) {
    if (!kernel_address_request.response) {
        kpanic(nullptr, "Limine didn't return kernel address");
    }

#ifdef DEBUG
    printf("[DEBUG] Allocating page for PML4\n");
#endif
    hypervisor_page_map.cr3_paddr = (uintptr_t)pmm_alloc_pages(1);
    if (!hypervisor_page_map.cr3_paddr) {
        kpanic(nullptr, "Failed to claim page for PML4");
    }

    hypervisor_page_map.pml4_address = hypervisor_page_map.cr3_paddr + hhdm->offset;
    memset((void *)hypervisor_page_map.pml4_address, 0, PAGE_SIZE);

    for ( size_t i = 0; i < memmap->entry_count; i++ ) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        uintptr_t base = entry->base;
        uintptr_t limit = base + entry->length;
        uint64_t flags = PTE_BIT_PRESENT;

        if ( entry->type == LIMINE_MEMMAP_USABLE ) {
            flags |= PTE_BIT_READ_WRITE;
        } else {
            flags |= PTE_BIT_EXECUTE_DISABLE;
        }

        for ( uintptr_t addr = base; addr < limit; addr += PAGE_SIZE ) {
            map( addr, addr, flags );
        }
    }

    uintptr_t text_start = ALIGN_DOWN((uintptr_t)text_start_addr, PAGE_SIZE),
              rodata_start = ALIGN_DOWN((uintptr_t)rodata_start_addr, PAGE_SIZE),
              data_start = ALIGN_DOWN((uintptr_t)data_start_addr, PAGE_SIZE),
              text_end = ALIGN_UP((uintptr_t)text_end_addr, PAGE_SIZE),
              rodata_end = ALIGN_UP((uintptr_t)rodata_end_addr, PAGE_SIZE),
              data_end = ALIGN_UP((uintptr_t)data_end_addr, PAGE_SIZE);
    kernel_address = kernel_address_request.response;

    for ( uintptr_t addr = text_start; addr < text_end; addr += PAGE_SIZE ) {
        map( addr + hhdm->offset, addr, PTE_BIT_PRESENT );
    }

    for ( uintptr_t addr = rodata_start; addr < rodata_end; addr += PAGE_SIZE ) {
        map( addr + hhdm->offset, addr, PTE_BIT_PRESENT | PTE_BIT_EXECUTE_DISABLE );
    }

    for ( uintptr_t addr = data_start; addr < data_end; addr += PAGE_SIZE ) {
        map( addr + hhdm->offset, addr, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_EXECUTE_DISABLE );
    }

    load_context( );
}

static uint64_t *get_below_pml( uint64_t *pml_pointer, uint64_t index, bool force )
{
    if ( ( pml_pointer[index] & PTE_BIT_PRESENT ) != 0 ) {
        return (uint64_t *)( ( pml_pointer[index] & 0x000ffffffffff000 ) + hhdm->offset );
    }
    if ( !force ) {
        return nullptr;
    }

    void *below_pml = pmm_alloc_pages(1);
    if ( below_pml == nullptr ) {
        kpanic(nullptr, "Failed to allocate page for lower level page table");
    }
    memset((void *)( (uint64_t)below_pml + hhdm->offset ), 0, PAGE_SIZE);

    pml_pointer[index] = (uint64_t)below_pml | PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_ACCESS_ALL;
    return (uint64_t *)( (uint64_t)below_pml + hhdm->offset );
}

static uint64_t *pml4_to_pt( uint64_t *pml4, uint64_t va, bool force )
{
    size_t pml4_index = ( va & ( 0x1fful << 39 ) ) >> 39,
           pdpt_index = ( va & ( 0x1fful << 30 ) ) >> 30,
           pd_index = ( va & ( 0x1fful << 21 ) ) >> 21;

    uint64_t *pdpt = nullptr,
             *pd = nullptr,
             *pt = nullptr;

    pdpt = get_below_pml( pml4, pml4_index, force );
    if ( !pdpt ) {
        return nullptr;
    }
    pd = get_below_pml( pdpt, pdpt_index, force );
    if ( !pd ) {
        return nullptr;
    }
    pt = get_below_pml( pd, pd_index, force );
    if ( !pt ) {
        return nullptr;
    }

    return pt;
}

void map(uintptr_t va, uintptr_t pa, uint64_t flags)
{
    spinlock_lock(&mmu_lock);
    size_t pt_index = (va & (0x1fful << 12)) >> 12;
    uint64_t *pt = pml4_to_pt((uint64_t *)hypervisor_page_map.pml4_address, va, true);
    if (pt == nullptr) {
        kpanic(nullptr, "Page table doesn't exist and didn't get created?");
    }
    pt[pt_index] = pa | flags;

    tlb_flush();

    spinlock_unlock(&mmu_lock);
}

bool unmap( uintptr_t va, bool free_pa )
{
    spinlock_lock(&mmu_lock);
    size_t pt_index = (va & (0x1fful << 12)) >> 12;
    uint64_t *pt = pml4_to_pt((uint64_t *)hypervisor_page_map.pml4_address, va, false);
    if (pt == nullptr) {
        spinlock_unlock(&mmu_lock);
        return false;
    }

    if (free_pa) {
        pmm_free_pages((void *)(pt[pt_index] & 0x000ffffffffff000), 1);
    }

    pt[pt_index] = 0;

    tlb_flush( );

    spinlock_unlock(&mmu_lock);

    return true;
}

void load_context( )
{
    __asm__ volatile (
        "movq %0, %%cr3\n"
        : : "r" ( (uint64_t *)( hypervisor_page_map.cr3_paddr ) ) : "memory"
    );
}
