#include <mm/vmm.h>
#include <kprintf.h>
#include <x86/smp.h>
#include <x86/idt.h>
#include <mem.h>

pageCTX kernelPmc = { 0x0 };
static uint64_t phys_addr_width = 0;
static uint64_t lin_addr_width = 0;

struct limine_kernel_address_response* responseVoid;

typedef char linkerSym[];

struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
    .response = responseVoid
};

struct limine_kernel_address_response *kernel_address;

static klockc map_page_lock;

static uint64_t *get_below_pml(uint64_t *pml_pointer, uint64_t index, bool force);
static uint64_t *pml4_to_pt(uint64_t *pml4, uint64_t va, bool force);

static inline void tlb_flush()
{
    asm volatile (
        "movq %%cr3, %%rax\n\
	    movq %%rax, %%cr3\n"
        : : : "rax"
   );
}

static uint64_t *get_below_pml(uint64_t *pml_pointer, uint64_t index, bool force)
{
    if ((pml_pointer[index] & PTE_BIT_PRESENT) != 0) {
        return (uint64_t *)((pml_pointer[index] & 0x000ffffffffff000) + hhdm->offset);
    }

    if (!force) {
        return NULL;
    }

    void *below_pml = allocate_pages(1);
    if (below_pml == NULL) {
        debugf("Failed to allocate pml4");
                kpanic();
    }
    memset((void *)((uint64_t)below_pml + hhdm->offset), 0, PAGE_SIZE);

    pml_pointer[index] = (uint64_t)below_pml | PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_ACCESS_ALL;
    return (uint64_t *)((uint64_t)below_pml + hhdm->offset);
}

static uint64_t *pml4_to_pt(uint64_t *pml4, uint64_t va, bool force)
{
    size_t pml4_index = (va & (0x1fful << 39)) >> 39,
        pdpt_index = (va & (0x1fful << 30)) >> 30,
        pd_index = (va & (0x1fful << 21)) >> 21;

    uint64_t *pdpt = NULL,
        *pd = NULL,
        *pt = NULL;

    pdpt = get_below_pml(pml4, pml4_index, force);
    if (!pdpt) {
        return NULL;
    }
    pd = get_below_pml(pdpt, pdpt_index, force);
    if (!pd) {
        return NULL;
    }
    pt = get_below_pml(pd, pd_index, force);
    if (!pt) {
        return NULL;
    }

    return pt;
}

void initVmm(void)
{
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    asm volatile ("cpuid" : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx) : "a"(1));
    phys_addr_width = ecx & 0xFF;
    lin_addr_width = (ecx >> 8) & 0xFF;
    debugf("Physical address width: 0x%x, Linear address width: 0x%x", phys_addr_width, lin_addr_width);

    if (!kernel_address_request.response) {
        debugf("Failed to get kernel address");
                kpanic();
    }

    kernelPmc.pml4_address = (uintptr_t)allocate_pages(1);
    if (!kernelPmc.pml4_address) {
        debugf("Failed to allocate pml4");
                kpanic();
    }
    kernelPmc.pml4_address += hhdm->offset;
    memset((void *)kernelPmc.pml4_address, 0, PAGE_SIZE);

    extern linkerSym textStartAddr, textEndAddr,
        rodataStartAddr, rodataEndAddr,
        dataStartAddr, dataEndAddr;
    
    uintptr_t text_start = ALIGN_DOWN((uintptr_t)textStartAddr, PAGE_SIZE),
        rodata_start = ALIGN_DOWN((uintptr_t)rodataStartAddr, PAGE_SIZE),
        data_start = ALIGN_DOWN((uintptr_t)dataStartAddr, PAGE_SIZE),
        text_end = ALIGN_UP((uintptr_t)textEndAddr, PAGE_SIZE),
        rodata_end = ALIGN_UP((uintptr_t)rodataEndAddr, PAGE_SIZE),
        data_end = ALIGN_UP((uintptr_t)dataEndAddr, PAGE_SIZE);
    
    if (!kernel_address_request.response) {
        debugf("Failed to get kernel address");
                kpanic();
    }

    struct limine_kernel_address_response *ka = kernel_address_request.response;

    for (size_t off = 0; off < ALIGN_UP(total_pmm_struct_bytes, PAGE_SIZE); off += PAGE_SIZE) {
        mapPage(&kernelPmc, (uintptr_t)page_bitmap + off, (uintptr_t)page_bitmap - hhdm->offset + off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    }

        for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (size_t off = entry->base; off < entry->base + entry->length; off += PAGE_SIZE) {
                mapPage(&kernelPmc, off + hhdm->offset, off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
            }
        }
        else if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER) {
            for (size_t off = 0; off < ALIGN_UP(entry->base + entry->length, PAGE_SIZE); off += PAGE_SIZE) {
                uintptr_t base_off_aligned = ALIGN_UP(entry->base + off, PAGE_SIZE);
                mapPage(&kernelPmc, base_off_aligned + hhdm->offset, base_off_aligned,
                    PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_EXECUTE_DISABLE | PTE_BIT_WRITE_THROUGH_CACHING);
            }
        }
        else if (entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            for (size_t off = 0; off < ALIGN_UP(entry->base + entry->length, PAGE_SIZE); off += PAGE_SIZE) {
                mapPage(&kernelPmc, entry->base + off + hhdm->offset, entry->base + off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
            }
        }
    }

    for (uintptr_t text_addr = text_start; text_addr < text_end; text_addr += PAGE_SIZE) {
        uintptr_t phys = text_addr - ka->virtual_base + ka->physical_base;
        mapPage(&kernelPmc, text_addr, phys, PTE_BIT_PRESENT);
    }

    for (uintptr_t rodata_addr = rodata_start; rodata_addr < rodata_end; rodata_addr += PAGE_SIZE) {
        uintptr_t phys = rodata_addr - ka->virtual_base + ka->physical_base;
        mapPage(&kernelPmc, rodata_addr, phys, PTE_BIT_PRESENT | PTE_BIT_EXECUTE_DISABLE);
    }

    for (uintptr_t data_addr = data_start; data_addr < data_end; data_addr += PAGE_SIZE) {
        uintptr_t phys = data_addr - ka->virtual_base + ka->physical_base;
        mapPage(&kernelPmc, data_addr, phys, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_EXECUTE_DISABLE);
    }
    
    setCFX(&kernelPmc);
    tlb_flush();
}

void mapPage(pageCTX *pmc, uintptr_t va, uintptr_t pa, uint64_t flags)
{
    acquire_lock(&map_page_lock);
    size_t pt_index = EXTRACT_BITS(va, 12ul, 20ul);
    uint64_t *pt = pml4_to_pt((uint64_t *)pmc->pml4_address, va, true);
    if (pt == NULL) {
        debugf("Page Mapping couldnt be made (pt doesn't exist and didnt get created)");
                kpanic();
        __builtin_unreachable();
    }
    pt[pt_index] = pa | flags;

    tlb_flush();

    release_lock(&map_page_lock);
}

bool unmapPage(pageCTX *pmc, uintptr_t va, bool free_pa)
{
    acquire_lock(&map_page_lock);
    size_t pt_index = (va & (0x1fful << 12)) >> 12;
    uint64_t *pt = pml4_to_pt((uint64_t *)pmc->pml4_address, va, false);
    if (pt == NULL) {
        release_lock(&map_page_lock);
        return false;
    }

    if (free_pa) {
        free_pages((void *)(pt[pt_index] & 0x000ffffffffff000), 1);
    }

    pt[pt_index] = 0;

    tlb_flush();

    release_lock(&map_page_lock);

    return true;
}

uintptr_t virt2phys(pageCTX *pmc, uintptr_t virt)
{
    size_t pt_index = (virt & (0x1fful << 12)) >> 12;
    uint64_t *pt = pml4_to_pt((uint64_t *)pmc->pml4_address, virt, false);
    uint64_t mask = ((1ull << phys_addr_width) - 1) & ~0xFFF;

    return (pt[pt_index] & mask) | (virt & 0xFFF);
}

inline void setCFX(const pageCTX *pmc)
{
    __asm__ volatile (
        "movq %0, %%cr3\n"
        : : "r" ((uint64_t *)(pmc->pml4_address - hhdm->offset)) : "memory"
    );
}