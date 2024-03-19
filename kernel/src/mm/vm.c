#include <mm/vm.h>
#include <binary.h>
#include <mem.h>
#include <string.h>
#include <debug.h>
#include <mm/page.h>


typedef char linker_symbol_ptr[];
PageMapCtx kernelPmc = { 0x0 };

struct limine_kernel_address_request kernelAddressRequest = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};
struct limine_kernel_address_response *kernelAddress;


static inline void tlbFlush()
{
    asm volatile (
        "movq %%cr3, %%rax\n\
	    movq %%rax, %%cr3\n"
        : : : "rax"
   );
}

static uint64_t* getBelowPml(uint64_t* pmlPointer, uint64_t index, bool force)
{
    if ((pmlPointer[index] & PTE_BIT_PRESENT) != 0) {
        return (uint64_t *)((pmlPointer[index] & 0x000ffffffffff000) + hhdm->offset);
    }
    if (!force) {
        return NULL;
    }

    void *belowPml = pgmClaimPage(1);
    if (belowPml == NULL) {
        log_crit("Virtual Memory Manager", "Failed to allocate page for PML%d", index);
        asm volatile("cli\nhlt");
    }
    memset((void *)((uint64_t)belowPml + hhdm->offset), 0, PAGE_SIZE);
    pmlPointer[index] = (uint64_t)belowPml | PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_ACCESS_ALL;
    return (uint64_t *)((uint64_t)belowPml + hhdm->offset);
}

static uint64_t* pmlToPt(uint64_t* pml4, uint64_t va, bool force)
{
    size_t pml4Index = (va & (0x1fful << 39)) >> 39,
        pdptIndex = (va & (0x1fful << 30)) >> 30,
        pdIndex = (va & (0x1fful << 21)) >> 21;

    uint64_t *pdpt = NULL,
        *pd = NULL,
        *pt = NULL;

    pdpt = getBelowPml(pml4, pml4Index, force);
    if (!pdpt) {
        return NULL;
    }
    pd = getBelowPml(pdpt, pdptIndex, force);
    if (!pd) {
        return NULL;
    }
    pt = getBelowPml(pd, pdIndex, force);
    if (!pt) {
        return NULL;
    }

    return pt;
}

void initVm(void)
{
    
    if (!kernelAddressRequest.response) {
        log_crit("Virtual Memory Manager", "limine kernel address request not answered");
        asm volatile ("cli\n hlt\n");
    }
    
    kernelPmc.pml4Address = (uintptr_t)pgmClaimPage(1);
    if (!kernelPmc.pml4Address) {
        log_crit("Virtual Memory Manager", "Failed to allocate page for PML4");
        asm volatile ("cli\n hlt\n");
    }

    kernelPmc.pml4Address += hhdm->offset;
    memset((void *)kernelPmc.pml4Address, 0, PAGE_SIZE);

    extern linker_symbol_ptr textStartAddr, textEndAddr,
        rodataStartAddr, rodataEndAddr,
        dataStartAddr, dataEndAddr;

    uintptr_t textStart = ALIGN_DOWN((uintptr_t)textStartAddr, PAGE_SIZE),
        rodataStart = ALIGN_DOWN((uintptr_t)rodataStartAddr, PAGE_SIZE),
        dataStart = ALIGN_DOWN((uintptr_t)dataStartAddr, PAGE_SIZE),
        textEnd = ALIGN_UP((uintptr_t)textEndAddr, PAGE_SIZE),
        rodataEnd = ALIGN_UP((uintptr_t)rodataEndAddr, PAGE_SIZE),
        dataEnd = ALIGN_UP((uintptr_t)dataEndAddr, PAGE_SIZE);
    
    struct limine_kernel_address_response *ka = kernelAddressRequest.response;

    // vmMapSp(&kernelPmc, ALIGN_DOWN((lapic_address + hhdm->offset), PAGE_SIZE),
    //     ALIGN_DOWN(lapic_address, PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    // lapic_address += hhdm->offset;

    for (size_t off = 0; off < ALIGN_UP(pageTotalBytesPageStructures, PAGE_SIZE); off += PAGE_SIZE) {
        
        vmMapSp(&kernelPmc, (uintptr_t)pageBitmap + off, (uintptr_t)pageBitmap - hhdm->offset + off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    }

    log_debug("Virtual Memory Manager", "Memmap entry count: %lu", memmap->entry_count);
    for (size_t i = 0; i < memmap->entry_count; i++) {
        
        struct limine_memmap_entry *entry = memmap->entries[i];
        log_debug("Virtual Memory Manager", "Entry %-2lu: Base = 0x%x, End = 0x%x, Length = %lu bytes, Type = %lu",
            i, entry->base, entry->base + entry->length, entry->length, entry->type);
        
        
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (size_t off = entry->base; off < entry->base + entry->length; off += PAGE_SIZE) {
                vmMapSp(&kernelPmc, off + hhdm->offset, off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
            }
        }
        else if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER) {
            for (size_t off = 0; off < ALIGN_UP(entry->base + entry->length, PAGE_SIZE); off += PAGE_SIZE) {
                uintptr_t baseOffAligned = ALIGN_UP(entry->base + off, PAGE_SIZE);
                vmMapSp(&kernelPmc, baseOffAligned + hhdm->offset, baseOffAligned, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_EXECUTE_DISABLE);
            }
        }
        else if (entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            for (size_t off = 0; off < ALIGN_UP(entry->base + entry->length, PAGE_SIZE); off += PAGE_SIZE) {
                vmMapSp(&kernelPmc, entry->base + off + hhdm->offset, entry->base + off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
            }
        }
    }

    for (uintptr_t textAddr = textStart; textAddr < textEnd; textAddr += PAGE_SIZE) {
        uintptr_t phys = textAddr - ka->virtual_base + ka->physical_base;
        vmMapSp(&kernelPmc, textAddr, phys, PTE_BIT_PRESENT);
    }
    
    for (uintptr_t rodataAddr = rodataStart; rodataAddr < rodataEnd; rodataAddr += PAGE_SIZE) {
        uintptr_t phys = rodataAddr - ka->virtual_base + ka->physical_base;
        vmMapSp(&kernelPmc, rodataAddr, phys, PTE_BIT_PRESENT | PTE_BIT_EXECUTE_DISABLE);
    }

    for (uintptr_t dataAddr = dataStart; dataAddr < dataEnd; dataAddr += PAGE_SIZE) {
        uintptr_t phys = dataAddr - ka->virtual_base + ka->physical_base;
        vmMapSp(&kernelPmc, dataAddr, phys, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_EXECUTE_DISABLE);
    }
    log_debug("Virtual Memory Manager", "Kernel Address Request: %p", kernelAddressRequest.response);

    vmSetCFX(&kernelPmc);
    tlbFlush();
}

void vmMapSp(PageMapCtx *pmc, uintptr_t va, uintptr_t pa, uint64_t flags)
{
    size_t ptIndex = EXTRACT_BITS(va, 12ul, 20ul);
    uint64_t *pt = pmlToPt((uint64_t *)pmc->pml4Address, va, true);
    if (pt == NULL) {
        log_crit("Virtual Memory Manager", "Failed to allocate page table for VA 0x%016lX", va);
        asm volatile("cli\nhlt");
    }
    pt[ptIndex] = pa | flags;
    tlbFlush();
}

void vmUnmapSp(PageMapCtx *pmc, uintptr_t va, bool freePa)
{
    size_t ptIndex = (va & (0x1fful << 12)) >> 12;
    uint64_t *pt = pmlToPt((uint64_t *)pmc->pml4Address, va, false);
    if (pt == NULL) {
        log_crit("Virtual Memory Manager", "Failed to find page table for VA 0x%016lX", va);
        asm volatile("cli\nhlt");
    }
    if (freePa) {
        pgmFreePage((void *)(pt[ptIndex] & 0x000ffffffffff000), 1);
    }
    pt[ptIndex] = 0x0;
    tlbFlush();
}

void vmSetCFX(const PageMapCtx *pmc)
{
    asm volatile (
        "movq %0, %%cr3\n"
        : : "r" ((uint64_t *)(pmc->pml4Address - hhdm->offset)) : "memory"
    );
}