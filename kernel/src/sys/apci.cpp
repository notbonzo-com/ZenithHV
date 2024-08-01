#include <sys/apci.hpp>
#include <sys/mm/mmu.hpp>
#include <sys/mm/pmm.hpp>
#include <sys/idt.hpp>
#include <kmalloc>
#include <kprintf>
#include <new>
#include <vector>
#include <atomic>
#include <util>

namespace apci {

static bool xsdt_present = false;

volatile struct RSDP *rsdp_ptr = NULL;
volatile struct RSDT *rsdt_ptr = NULL;

volatile struct FADT *fadt_ptr = NULL;
volatile struct MADT *madt_ptr = NULL;
volatile struct MCFG *mcfg_ptr = NULL;

struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
	.response = NULL
};

std::vector<IOAPIC*> ioapics;
std::vector<LAPIC*> lapics;
std::vector<ISO*> isos;

static bool validate_table(volatile struct SDTHeader *table_header)
{
    uint8_t sum = 0;

    for (size_t i = 0; i < table_header->length; i++) {
        sum += ((uint8_t *)table_header)[i];
    }

    return (sum == 0);
}

void parse(void)
{
	kprintf(" -> Parsing the ACPI tables...\n");
	if (rsdp_request.response == NULL || rsdp_request.response->address == NULL) {
        intr::kpanic(NULL, "ACPI is not supported");
    }
	rsdp_ptr = (struct RSDP*)rsdp_request.response->address;
    mmu::map(&mmu::kernel_pmc, ALIGN_DOWN((uintptr_t)rsdp_ptr, PAGE_SIZE),
        ALIGN_DOWN(((uintptr_t)rsdp_ptr - pmm::hhdm->offset), PAGE_SIZE), mmu::PTE_BIT_PRESENT | mmu::PTE_BIT_READ_WRITE);

    xsdt_present = (rsdp_ptr->revision >= 2) ? true : false;

    rsdt_ptr = xsdt_present ? (struct RSDT *)((uintptr_t)rsdp_ptr->xsdt_address + pmm::hhdm->offset) : (struct RSDT *)((uintptr_t)rsdp_ptr->rsdt_address + pmm::hhdm->offset);
    if (rsdt_ptr == NULL) {
        intr::kpanic(NULL, "ACPI is not supported");
    }

    mmu::map(&mmu::kernel_pmc, ALIGN_DOWN((uintptr_t)rsdt_ptr, PAGE_SIZE),
        ALIGN_DOWN(((uintptr_t)rsdt_ptr - pmm::hhdm->offset), PAGE_SIZE), mmu::PTE_BIT_PRESENT | mmu::PTE_BIT_READ_WRITE);

    size_t entry_count = (rsdt_ptr->header.length - sizeof(struct RSDT)) / (xsdt_present ? 8 : 4);

    for (size_t i = 0; i < entry_count; i++) {
        struct SDTHeader *head = NULL;

        if (xsdt_present) {
            uint32_t *xsdt_table = (uint32_t *)((uintptr_t)rsdt_ptr + sizeof(struct SDTHeader));
            size_t head_lo = xsdt_table[i * 2];
            size_t head_hi = xsdt_table[i * 2 + 1];
            head =  (struct SDTHeader *)(((head_hi << 32) | head_lo) + pmm::hhdm->offset);
        } else {
            uint32_t *rsdt_table = (uint32_t *)((uintptr_t)rsdt_ptr + sizeof(struct SDTHeader));
            head = (struct SDTHeader *)(rsdt_table[i] + pmm::hhdm->offset);
        }

        mmu::map(&mmu::kernel_pmc, ALIGN_DOWN((uintptr_t)head, PAGE_SIZE),
            ALIGN_DOWN(((uintptr_t)head - pmm::hhdm->offset), PAGE_SIZE), mmu::PTE_BIT_PRESENT | mmu::PTE_BIT_READ_WRITE);
    }


    if (!(fadt_ptr = (FADT*)get_sdt("FACP")) || !validate_table(&fadt_ptr->header)) {
        intr::kpanic(NULL, "FADT not found\n");
    }
    if (!(madt_ptr = (MADT*)get_sdt("APIC")) || !validate_table(&madt_ptr->header)) {
        intr::kpanic(NULL, "MADT not found\n");
    }
    if (!(mcfg_ptr = (MCFG*)get_sdt("MCFG"))) {
        intr::kpanic(NULL, "MCFG not found\n");
    }

    parse_madt(madt_ptr);
}

void *get_sdt(const char signature[4])
{
    size_t entry_count = (rsdt_ptr->header.length - sizeof(struct SDTHeader)) / (xsdt_present ? 8 : 4);

    for (size_t i = 0; i < entry_count; i++) {
        struct SDTHeader *head;

        if (xsdt_present) {
            uint32_t *xsdt_table = (uint32_t *)((uintptr_t)rsdt_ptr + sizeof(struct SDTHeader));
            size_t head_lo = xsdt_table[i * 2];
            size_t head_hi = xsdt_table[i * 2 + 1];
            head = (struct SDTHeader *)(((head_hi << 32) | head_lo) + pmm::hhdm->offset);
        } else {    // rsdt
            uint32_t *rsdt_table = (uint32_t *)((uintptr_t)rsdt_ptr + sizeof(struct SDTHeader));
            head = (struct SDTHeader *)(rsdt_table[i] + pmm::hhdm->offset);
        }

        if (!std::memcmp(head->signature, signature, 4)) {
            kprintf(" -> %4s found at (pa) 0x%p of length %-u\n",
                signature, (uintptr_t)head - pmm::hhdm->offset, head->length);
            return head;
        }
    }

    return NULL;
}

void parse_madt(volatile struct MADT *madt)
{
    for (uintptr_t off = 0; off < madt->header.length - sizeof(struct MADT); ) {
        struct MADTHeader *madt_hdr = (struct MADTHeader *)(madt->entries + off);
        if (madt_hdr->type == MADT_ENTRY_PROCESSOR_LAPIC) {
            lapics.push_back((LAPIC*)madt_hdr);
        }
        else if (madt_hdr->type == MADT_ENTRY_IO_APIC) {
            ioapics.push_back((IOAPIC*)madt_hdr);
        }
        else if (madt_hdr->type == MADT_ENTRY_INTERRUPT_SOURCE_OVERRIDE) {
            isos.push_back((ISO*)madt_hdr);
        }

        off += madt_hdr->length;
    }

    kprintf(" -> Found %llu ioapic(s) and %llu lapic(s)\n", ioapics.size(), lapics.size());
    if (ioapics.size() == 0) {
        intr::kpanic(NULL, "Systems without an IOAPIC are not supported!\n");
    }
}

}
