/**
 * Credit to Rüdiger from https://github.com/Ruediga/MunkOS/blob/main/kernel/src/acpi.c
*/
#include <x86/int.h>
#include <x86/acpi.h>
#include <mm/kalloc.h>
#include <mm/vm.h>
#include <mm/page.h>
#include <mem.h>
#include <binary.h>
#include <debug.h>
#include <stdio.h>

VECTOR_TMPL_TYPE(acpi_ioapic_ptr)
VECTOR_TMPL_TYPE(acpi_lapic_ptr)
VECTOR_TMPL_TYPE(acpi_iso_ptr)

bool xsdt_present;

volatile struct acpi_rsdp *rsdp_ptr = NULL;
volatile struct acpi_rsdt *rsdt_ptr = NULL;

volatile struct acpi_fadt *fadt_ptr = NULL;
volatile struct acpi_madt *madt_ptr = NULL;
volatile struct acpi_mcfg *mcfg_ptr = NULL;

struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

vector_acpi_ioapic_ptr_t ioapics = VECTOR_INIT(acpi_ioapic_ptr);
vector_acpi_lapic_ptr_t lapics = VECTOR_INIT(acpi_lapic_ptr);
vector_acpi_iso_ptr_t isos = VECTOR_INIT(acpi_iso_ptr);

static bool validate_table(volatile struct acpi_sdt_header *table_header)
{
    uint8_t sum = 0;

    for (size_t i = 0; i < table_header->length; i++) {
        sum += ((uint8_t *)table_header)[i];
    }

    return (sum == 0);
}

void parse_acpi(void)
{
    if (rsdp_request.response == NULL || rsdp_request.response->address == NULL) {
        panic(NULL, 0, "ACPI is not supported\n");
    }

    rsdp_ptr = rsdp_request.response->address;
    vmMapSp(&kernelPmc, ALIGN_DOWN((uintptr_t)rsdp_ptr, PAGE_SIZE),
        ALIGN_DOWN(((uintptr_t)rsdp_ptr - hhdm->offset), PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);

    xsdt_present = (rsdp_ptr->revision >= 2) ? true : false;
    rsdt_ptr = xsdt_present ? (struct acpi_rsdt *)((uintptr_t)rsdp_ptr->xsdt_address + hhdm->offset)
        : (struct acpi_rsdt *)((uintptr_t)rsdp_ptr->rsdt_address + hhdm->offset);
    if (rsdt_ptr == NULL) {
        panic(NULL, 0, "ACPI is not supported\n");
    }
    vmMapSp(&kernelPmc, ALIGN_DOWN((uintptr_t)rsdt_ptr, PAGE_SIZE),
        ALIGN_DOWN(((uintptr_t)rsdt_ptr - hhdm->offset), PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);

    size_t entry_count = (rsdt_ptr->header.length - sizeof(struct acpi_sdt_header)) / (xsdt_present ? 8 : 4);
    for (size_t i = 0; i < entry_count; i++) {
        struct acpi_sdt_header *head = NULL;
        head = (struct acpi_sdt_header *)(xsdt_present ? (((uint64_t *)(rsdt_ptr->ptr))[i] + hhdm->offset)
            : ((((uint32_t *)(rsdt_ptr->ptr))[i]) + hhdm->offset));
        vmMapSp(&kernelPmc, ALIGN_DOWN((uintptr_t)head, PAGE_SIZE),
            ALIGN_DOWN(((uintptr_t)head - hhdm->offset), PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    }

    if (!(fadt_ptr = get_sdt("FACP")) || !validate_table(&fadt_ptr->header)) {
        panic(NULL, 0, "FADT not found\n");
    }
    if (!(madt_ptr = get_sdt("APIC")) || !validate_table(&madt_ptr->header)) {
        panic(NULL, 0, "MADT not found\n");
    }
    if (!(mcfg_ptr = get_sdt("MCFG"))) {
        panic(NULL, 0, "MCFG not found\n");
    }

    parse_madt(madt_ptr);
}

void *get_sdt(const char signature[static 4])
{
    size_t entry_count = (rsdt_ptr->header.length - sizeof(struct acpi_sdt_header)) / (xsdt_present ? 8 : 4);

    for (size_t i = 0; i < entry_count; i++) {
        struct acpi_sdt_header *head = (xsdt_present ?
            (struct acpi_sdt_header *)(((uint64_t *)rsdt_ptr->ptr)[i] + hhdm->offset)
            : (struct acpi_sdt_header *)(((uint32_t *)rsdt_ptr->ptr)[i] + hhdm->offset));

        if (!memcmp(head->signature, signature, 4)) {
            log_debug("ACPI", "%s found at (pa) 0x%p of length %u\n",
                signature, (uintptr_t)head - hhdm->offset, head->length);
            return head;
        }
    }

    return NULL;
}

void parse_madt(volatile struct acpi_madt *madt)
{
    for (uintptr_t off = 0; off < madt->header.length - sizeof(struct acpi_madt); ) {
        struct acpi_madt_header *madt_hdr = (struct acpi_madt_header *)(madt->entries + off);
        if (madt_hdr->lcst_id == MADT_ENTRY_PROCESSOR_LAPIC) {
            lapics.push_back(&lapics, (acpi_lapic_ptr)madt_hdr);
        }
        else if (madt_hdr->lcst_id == MADT_ENTRY_IO_APIC) {
            ioapics.push_back(&ioapics, (acpi_ioapic_ptr)madt_hdr);
        }
        else if (madt_hdr->lcst_id == MADT_ENTRY_INTERRUPT_SOURCE_OVERRIDE) {
            isos.push_back(&isos, (acpi_iso_ptr)madt_hdr);
        }

        off += madt_hdr->length;
    }

    log_debug("ACPI", "found %lu ioapic(s) and %lu lapic(s)\n", ioapics.size, lapics.size);
    if (ioapics.size == 0) {
        panic(NULL, 0, "Systems without an IOAPIC are not supported!\n");
    }
}