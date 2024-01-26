#include "vm.h"
#include <binary.h>
#include <mem.h>
#include <string.h>
#include <drivers/debug/e9.h>
#include <drivers/mm/pg.h>

typedef char linker_symbol_ptr[];
page_map_ctx kernel_pmc = { 0x0 };

struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};
struct limine_kernel_address_response *kernel_address;


static inline void TlbFlush()
{
    asm volatile (
        "movq %%cr3, %%rax\n\
	    movq %%rax, %%cr3\n"
        : : : "rax"
   );
}

static uint64_t* GetBelowPml(uint64_t* pml_pointer, uint64_t index, bool force)
{
    if ((pml_pointer[index] & PTE_BIT_PRESENT) != 0) {
        return (uint64_t *)((pml_pointer[index] & 0x000ffffffffff000) + hhdm->offset);
    }
    if (!force) {
        return NULL;
    }

    void *below_pml = x86_64_pgm_claim_page(1);
    if (below_pml == NULL) {
        log_crit("Virtual Memory Manager", "Failed to allocate page for PML%d", index);
        asm volatile("cli\nhlt");
    }
    memset((void *)((uint64_t)below_pml + hhdm->offset), 0, PAGE_SIZE);
    pml_pointer[index] = (uint64_t)below_pml | PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_ACCESS_ALL;
    return (uint64_t *)((uint64_t)below_pml + hhdm->offset);
}

static uint64_t* PmlToPt(uint64_t* pml4, uint64_t va, bool force)
{
    size_t pml4_index = (va & (0x1fful << 39)) >> 39,
        pdpt_index = (va & (0x1fful << 30)) >> 30,
        pd_index = (va & (0x1fful << 21)) >> 21;

    uint64_t *pdpt = NULL,
        *pd = NULL,
        *pt = NULL;

    pdpt = GetBelowPml(pml4, pml4_index, force);
    if (!pdpt) {
        return NULL;
    }
    //log_debug("Virtual Memory Manager", "pdpt: %p", pdpt);
    pd = GetBelowPml(pdpt, pdpt_index, force);
    if (!pd) {
        return NULL;
    }
    pt = GetBelowPml(pd, pd_index, force);
    if (!pt) {
        return NULL;
    }

    return pt;
}
void x86_64_VM_Init(void)
{
    
    if (!kernel_address_request.response) {
        log_crit("Virtual Memory Manager", "limine kernel address request not answered");
        asm volatile ("cli\n hlt\n");
    }
    
    kernel_pmc.pml4_address = (uintptr_t)x86_64_pgm_claim_page(1);
    if (!kernel_pmc.pml4_address) {
        log_crit("Virtual Memory Manager", "Failed to allocate page for PML4");
        asm volatile ("cli\n hlt\n");
    }

    kernel_pmc.pml4_address += hhdm->offset;
    memset((void *)kernel_pmc.pml4_address, 0, PAGE_SIZE);

    extern linker_symbol_ptr text_start_addr, text_end_addr,
        rodata_start_addr, rodata_end_addr,
        data_start_addr, data_end_addr;

    uintptr_t text_start = ALIGN_DOWN((uintptr_t)text_start_addr, PAGE_SIZE),
        rodata_start = ALIGN_DOWN((uintptr_t)rodata_start_addr, PAGE_SIZE),
        data_start = ALIGN_DOWN((uintptr_t)data_start_addr, PAGE_SIZE),
        text_end = ALIGN_UP((uintptr_t)text_end_addr, PAGE_SIZE),
        rodata_end = ALIGN_UP((uintptr_t)rodata_end_addr, PAGE_SIZE),
        data_end = ALIGN_UP((uintptr_t)data_end_addr, PAGE_SIZE);
    
    if (!kernel_address_request.response) {
        log_crit("Virtual Memory Manager", "limine kernel address request not answered");
        asm volatile("cli\n hlt\n");
    }
    struct limine_kernel_address_response *ka = kernel_address_request.response;

    // x86_64_VM_MapSP(&kernel_pmc, ALIGN_DOWN((lapic_address + hhdm->offset), PAGE_SIZE),
    //     ALIGN_DOWN(lapic_address, PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    // lapic_address += hhdm->offset;

    for (size_t off = 0; off < ALIGN_UP(pgm_total_bytes_pgm_structures, PAGE_SIZE); off += PAGE_SIZE) {
        
        x86_64_VM_MapSP(&kernel_pmc, (uintptr_t)pgm_page_bitmap + off, (uintptr_t)pgm_page_bitmap - hhdm->offset + off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    }

    log_debug("Virtual Memory Manager", "Memmap entry count: %lu", memmap->entry_count);
    for (size_t i = 0; i < memmap->entry_count; i++) {
        
        struct limine_memmap_entry *entry = memmap->entries[i];
        log_debug("Virtual Memory Manager", "Entry %-2lu: Base = 0x%016lX, End = 0x%016lX, Length = %lu bytes, Type = %lu",
            i, entry->base, entry->base + entry->length, entry->length, entry->type);
        
        
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (size_t off = entry->base; off < entry->base + entry->length; off += PAGE_SIZE) {
                x86_64_VM_MapSP(&kernel_pmc, off + hhdm->offset, off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
            }
        }
        else if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER) {
            for (size_t off = 0; off < ALIGN_UP(entry->base + entry->length, PAGE_SIZE); off += PAGE_SIZE) {
                uintptr_t base_off_aligned = ALIGN_UP(entry->base + off, PAGE_SIZE);
                x86_64_VM_MapSP(&kernel_pmc, base_off_aligned + hhdm->offset, base_off_aligned, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_EXECUTE_DISABLE);
            }
        }
        else if (entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            for (size_t off = 0; off < ALIGN_UP(entry->base + entry->length, PAGE_SIZE); off += PAGE_SIZE) {
                x86_64_VM_MapSP(&kernel_pmc, entry->base + off + hhdm->offset, entry->base + off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
            }
        }
    }
    // log_info("Virtual Memory Manager", "Break from memmap->entry_count loop");
    
    for (uintptr_t text_addr = text_start; text_addr < text_end; text_addr += PAGE_SIZE) {
        uintptr_t phys = text_addr - ka->virtual_base + ka->physical_base;
        x86_64_VM_MapSP(&kernel_pmc, text_addr, phys, PTE_BIT_PRESENT);
    }
    
    for (uintptr_t rodata_addr = rodata_start; rodata_addr < rodata_end; rodata_addr += PAGE_SIZE) {
        uintptr_t phys = rodata_addr - ka->virtual_base + ka->physical_base;
        x86_64_VM_MapSP(&kernel_pmc, rodata_addr, phys, PTE_BIT_PRESENT | PTE_BIT_EXECUTE_DISABLE);
    }

    for (uintptr_t data_addr = data_start; data_addr < data_end; data_addr += PAGE_SIZE) {
        uintptr_t phys = data_addr - ka->virtual_base + ka->physical_base;
        x86_64_VM_MapSP(&kernel_pmc, data_addr, phys, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_EXECUTE_DISABLE);
    }
    log_debug("Virtual Memory Manager", "Kernel Address Request: %p", kernel_address_request.response);

    x86_64_VM_SetCTX(&kernel_pmc);
    // log_debug("Virtual Memory Manager", "CTX set");
    TlbFlush();
}

void x86_64_VM_MapSP(page_map_ctx *pmc, uintptr_t va, uintptr_t pa, uint64_t flags)
{
    size_t pt_index = EXTRACT_BITS(va, 12ul, 20ul);
    uint64_t *pt = PmlToPt((uint64_t *)pmc->pml4_address, va, true);
    if (pt == NULL) {
        log_crit("Virtual Memory Manager", "Failed to allocate page table for VA 0x%016lX", va);
        asm volatile("cli\nhlt");
    }
    pt[pt_index] = pa | flags;
    TlbFlush();
}

void x86_64_VM_UnmapSP(page_map_ctx *pmc, uintptr_t va, bool free_pa)
{
    size_t pt_index = (va & (0x1fful << 12)) >> 12;
    uint64_t *pt = PmlToPt((uint64_t *)pmc->pml4_address, va, false);
    if (pt == NULL) {
        log_crit("Virtual Memory Manager", "Failed to find page table for VA 0x%016lX", va);
        asm volatile("cli\nhlt");
    }
    if (free_pa) {
        x86_64_pgm_free_page((void *)(pt[pt_index] & 0x000ffffffffff000), 1);
    }
    pt[pt_index] = 0x0;
    TlbFlush();
}

void x86_64_VM_SetCTX(const page_map_ctx *pmc)
{
    asm volatile (
        "movq %0, %%cr3\n"
        : : "r" ((uint64_t *)(pmc->pml4_address - hhdm->offset)) : "memory"
    );
}