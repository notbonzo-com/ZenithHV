#include <sys/mm/mmu.hh>
#include <sys/mm/pmm.hh>
#include <kprintf>
#include <atomic>
#include <cstring>
#include <sys/idt.hh>
#include <macro.hh>
#include <cpuid.hh>

namespace mmu {

struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0,
    .response = nullptr
};

extern "C" char text_start_addr[], text_end_addr[],
        rodata_start_addr[], rodata_end_addr[],
        data_start_addr[], data_end_addr[];
	
page_map_ctx kernel_pmc;
static uint64_t phys_addr_width = 0;
static uint64_t lin_addr_width = 0;

struct limine_kernel_address_response *kernel_address;

std::klock map_page_lock;

void page_map_ctx::init()
{
    kprintf(" -> Initializing Virtual Memory Manager\n");
    
    uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
    kprintf(" -> Getting CPUID for address widths\n");
    __get_cpuid(0x80000008, &eax, &ebx, &ecx, &edx);
    phys_addr_width = eax & 0xFF;
    lin_addr_width = (eax >> 8) & 0xFF;
    kprintf(" -> Physical Address Width: %lu / Linear Address Width: %lu\n", phys_addr_width, lin_addr_width);

    if (!kernel_address_request.response) {
        intr::kpanic(nullptr, "Limine didn't return kernel address");
    }

    kprintf(" -> Claiming page for PML4\n");
    kernel_pmc.pml4_address = (uintptr_t)pmm::claim(1);
    if (!kernel_pmc.pml4_address) {
        intr::kpanic(nullptr, "Failed to claim page for PML4");
    }
    kernel_pmc.pml4_address += pmm::hhdm->offset;
    kprintf(" -> Zeroing out contents of newly allocated page\n");
    std::memset((void *)kernel_pmc.pml4_address, 0, PAGE_SIZE);

    uintptr_t text_start = ALIGN_DOWN((uintptr_t)text_start_addr, PAGE_SIZE),
              rodata_start = ALIGN_DOWN((uintptr_t)rodata_start_addr, PAGE_SIZE),
              data_start = ALIGN_DOWN((uintptr_t)data_start_addr, PAGE_SIZE),
              text_end = ALIGN_UP((uintptr_t)text_end_addr, PAGE_SIZE),
              rodata_end = ALIGN_UP((uintptr_t)rodata_end_addr, PAGE_SIZE),
              data_end = ALIGN_UP((uintptr_t)data_end_addr, PAGE_SIZE);

    kernel_address = kernel_address_request.response;
    uintptr_t kernel_base = text_start;
    uintptr_t kernel_highest = (data_end > rodata_end) ? data_end : rodata_end;
    kprintf(" -> Kernel Base Address: 0x%llx\n", kernel_base);
    kprintf(" -> Kernel Highest Address: 0x%llx\n", kernel_highest);

    kprintf(" -> Mapping kernel PMM structures\n");
    for (size_t off = 0; off < ALIGN_UP(pmm::totalBytesPmmStructures, PAGE_SIZE); off += PAGE_SIZE) {
        map((uintptr_t)pmm::pageBitmap + off, (uintptr_t)pmm::pageBitmap - pmm::hhdm->offset + off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    }

    // kprintf(" -> Mapping lapic\n");
    // map(ALIGN_DOWN((lapic::lapic_address + pmm::hhdm->offset), PAGE_SIZE), ALIGN_DOWN(lapic::lapic_address, PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    // lapic::lapic_address += pmm::hhdm->offset;

    kprintf(" -> Mapping usable memory and framebuffer\n");
    for (size_t i = 0; i < pmm::memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = pmm::memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (size_t off = entry->base; off < entry->base + entry->length; off += PAGE_SIZE) {
                map(off + pmm::hhdm->offset, off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
            }
        }
        else if (entry->type == LIMINE_MEMMAP_FRAMEBUFFER) {
            for (size_t off = 0; off < ALIGN_UP(entry->base + entry->length, PAGE_SIZE); off += PAGE_SIZE) {
                uintptr_t base_off_aligned = ALIGN_UP(entry->base + off, PAGE_SIZE);
                map(base_off_aligned + pmm::hhdm->offset, base_off_aligned,
                    PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_EXECUTE_DISABLE | PTE_BIT_WRITE_THROUGH_CACHING);
            }
        }
        else if (entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            for (size_t off = 0; off < ALIGN_UP(entry->base + entry->length, PAGE_SIZE); off += PAGE_SIZE) {
                map(entry->base + off + pmm::hhdm->offset, entry->base + off, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
            }
        }
    }

    kprintf(" -> Mapping kernel text segments\n");
    for (uintptr_t text_addr = text_start; text_addr < text_end; text_addr += PAGE_SIZE) {
        uintptr_t phys = text_addr - kernel_address->virtual_base + kernel_address->physical_base;
        map(text_addr, phys, PTE_BIT_PRESENT);
    }
    kprintf(" -> Mapping the kernel rodata segment\n");
    for (uintptr_t rodata_addr = rodata_start; rodata_addr < rodata_end; rodata_addr += PAGE_SIZE) {
        uintptr_t phys = rodata_addr - kernel_address->virtual_base + kernel_address->physical_base;
        map(rodata_addr, phys, PTE_BIT_PRESENT | PTE_BIT_EXECUTE_DISABLE);
    }
    kprintf(" -> Mapping the kernel data segment\n");
    for (uintptr_t data_addr = data_start; data_addr < data_end; data_addr += PAGE_SIZE) {
        uintptr_t phys = data_addr - kernel_address->virtual_base + kernel_address->physical_base;
        map(data_addr, phys, PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_EXECUTE_DISABLE);
    }
}

uint64_t *page_map_ctx::get_below_pml(uint64_t *pml_pointer, uint64_t index, bool force)
{
    if ((pml_pointer[index] & PTE_BIT_PRESENT) != 0) {
        return (uint64_t *)((pml_pointer[index] & 0x000ffffffffff000) + pmm::hhdm->offset);
    }
    if (!force) {
        return nullptr;
    }

    void *below_pml = pmm::claim(1);
    if (below_pml == nullptr) {
        intr::kpanic(nullptr, "Failed to allocate page for lower level page table");
    }
    std::memset((void *)((uint64_t)below_pml + pmm::hhdm->offset), 0, PAGE_SIZE);

    pml_pointer[index] = (uint64_t)below_pml | PTE_BIT_PRESENT | PTE_BIT_READ_WRITE | PTE_BIT_ACCESS_ALL;
    return (uint64_t *)((uint64_t)below_pml + pmm::hhdm->offset);
}

uint64_t *page_map_ctx::pml4_to_pt(uint64_t *pml4, uint64_t va, bool force)
{
    size_t pml4_index = (va & (0x1fful << 39)) >> 39,
           pdpt_index = (va & (0x1fful << 30)) >> 30,
           pd_index = (va & (0x1fful << 21)) >> 21;

    uint64_t *pdpt = nullptr,
             *pd = nullptr,
             *pt = nullptr;

    pdpt = get_below_pml(pml4, pml4_index, force);
    if (!pdpt) {
        return nullptr;
    }
    pd = get_below_pml(pdpt, pdpt_index, force);
    if (!pd) {
        return nullptr;
    }
    pt = get_below_pml(pd, pd_index, force);
    if (!pt) {
        return nullptr;
    }

    return pt;
}

void page_map_ctx::map(uintptr_t va, uintptr_t pa, uint64_t flags)
{
    map_page_lock.a();
    size_t pt_index = (va & (0x1fful << 12)) >> 12;
    uint64_t *pt = this->pml4_to_pt((uint64_t *)this->pml4_address, va, true);
    if (pt == nullptr) {
        intr::kpanic(nullptr, "Page table doesn't exist and didn't get created?");
    }
    pt[pt_index] = pa | flags;

    tlb_flush();

    map_page_lock.r();
}

bool page_map_ctx::unmap(uintptr_t va, bool free_pa)
{
    map_page_lock.a();
    size_t pt_index = (va & (0x1fful << 12)) >> 12;
    uint64_t *pt = this->pml4_to_pt((uint64_t *)this->pml4_address, va, false);
    if (pt == nullptr) {
        map_page_lock.r();
        return false;
    }

    if (free_pa) {
        pmm::free((void *)(pt[pt_index] & 0x000ffffffffff000), 1);
    }

    pt[pt_index] = 0;

    tlb_flush();

    map_page_lock.r();

    return true;
}

uintptr_t virt2phys(page_map_ctx *pmc, uintptr_t virt)
{
    size_t pt_index = (virt & (0x1fful << 12)) >> 12;
    uint64_t *pt = page_map_ctx::pml4_to_pt((uint64_t *)pmc->pml4_address, virt, false);
    if (!pt) return (uintptr_t)nullptr;
    uint64_t mask = ((1ull << phys_addr_width) - 1) & ~0xFFF;

    return (pt[pt_index] & mask) | (virt & 0xFFF);
}

void page_map_ctx::switch2()
{
    __asm__ volatile (
        "movq %0, %%cr3\n"
        : : "r" ((uint64_t *)(this->pml4_address - pmm::hhdm->offset)) : "memory"
    );
}

} // namespace vmm
