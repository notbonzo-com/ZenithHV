#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <mem.h>
#include <debug.h>
#include <binary.h>
#include <vector.h>
#include <x86/int.h>
#include <mm/page.h>
#include <mm/vm.h>
#include <x86/acpi.h>

#include <stdio.h>

bool xsdt_present;

struct Rsdp *rsdpPtr = NULL;
struct Rsdt *rsdtPtr = NULL;

struct Fadt *fadtPtr = NULL;
struct Madt *madtPtr = NULL;

static vector_t vecIoapic;
static vector_t vecLapic;

struct Ioapic *ioapics;
struct Lapic *lapics;

size_t ioapicCount = 0;
size_t lapicCount = 0;

struct limine_rsdp_request limineRsdpRequest = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

void initAcpi(void)
{
    initVector(&vecLapic);
    initVector(&vecIoapic);
    if (limineRsdpRequest.response == NULL || limineRsdpRequest.response->address == NULL)
        panic(NULL, 0, "ACPI is not supported\n");

    rsdpPtr = limineRsdpRequest.response->address;
    vmMapSp(&kernelPmc, ALIGN_DOWN((uintptr_t)rsdpPtr, PAGE_SIZE),
        ALIGN_DOWN(((uintptr_t)rsdpPtr - hhdm->offset), PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);

    if (rsdpPtr->revision >= 2) {
        rsdtPtr = (struct Rsdt*)((uintptr_t)rsdpPtr->xsdtAddress + hhdm->offset);
    } else {
        rsdtPtr = (struct Rsdt*)((uintptr_t)rsdpPtr->rsdtAddress + hhdm->offset);
    }
    
    if (rsdtPtr == NULL) {
        panic(NULL, 0, "ACPI is not supported\n");
    }

    vmMapSp(&kernelPmc, ALIGN_DOWN((uintptr_t)rsdtPtr, PAGE_SIZE),
        ALIGN_DOWN(((uintptr_t)rsdtPtr - hhdm->offset), PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);

    size_t entry_count = (rsdtPtr->header.length - sizeof(struct SdtHeader)) / (xsdt_present ? 8 : 4);
    for (size_t i = 0; i < entry_count; i++) {
        struct SdtHeader *head = NULL;

        if (rsdpPtr->revision >= 2) {
            head = (struct SdtHeader *)(((uint64_t *)(rsdtPtr->ptr))[i] + hhdm->offset);
        } else {
            head = (struct SdtHeader *)((((uint32_t *)(rsdtPtr->ptr))[i]) + hhdm->offset);
        }

        vmMapSp(&kernelPmc, ALIGN_DOWN((uintptr_t)head, PAGE_SIZE),
            ALIGN_DOWN(((uintptr_t)head - hhdm->offset), PAGE_SIZE), PTE_BIT_PRESENT | PTE_BIT_READ_WRITE);
    }

    if (!(fadtPtr = sdtFind("FACD"))) {
        panic(NULL, 0, "FADT not valid\n");
    }
    if (!(madtPtr = sdtFind("ACPI"))) {
        panic(NULL, 0, "MADT not valid\n");
    }

    parseMADT(madtPtr);
    lapics = (struct Lapic *)vecLapic.data;
    ioapics = (struct Ioapic *)vecIoapic.data;
}

void *sdtFind(const char signature[static 4])
{
    size_t entry_count = (rsdtPtr->header.length - sizeof(struct SdtHeader)) / (xsdt_present ? 8 : 4);

    for (size_t i = 0; i < entry_count; i++) {
        struct SdtHeader *head = NULL;

        if (rsdpPtr->revision >= 2) {
            head = (struct SdtHeader *)(((uint64_t *)(rsdtPtr->ptr))[i] + hhdm->offset);
        } else {
            head = (struct SdtHeader *)((((uint32_t *)(rsdtPtr->ptr))[i]) + hhdm->offset);
        }

        if (memcmp(head->signature, signature, 4)) {
            return head;
        }
    }

    return NULL;
}

void parseMADT(struct Madt *madt)
{
    log_debug("ACPI", "Parsing MADT\n");
    for (uintptr_t off = 0; off < madt->header.length - sizeof(struct Madt); ) {
        struct MadtHeader *madtHdr = (struct MadtHeader *)(madt->entries + off);
        if (madtHdr->lcstId == MADT_ENTRY_PROCESSOR_LAPIC) {
            log_debug("ACPI", "Lapic length: %d, ID: 0x%x\n", madtHdr->length, madtHdr->lcstId);
            appendVector(&vecLapic, madtHdr, madtHdr->length);
            lapicCount++;
        }
        else if (madtHdr->lcstId == MADT_ENTRY_IO_APIC) {
            log_debug("ACPI", "IOAPIC length: %d, ID: 0x%x\n", madtHdr->length, madtHdr->lcstId);
            appendVector(&vecIoapic, madtHdr, madtHdr->length);
            ioapicCount++;
        }
        off += MAX(madtHdr->length, 2);
    }
    if (ioapicCount == 0) {
        panic(NULL, 0, "Systems without an IOAPIC are not supported!\n");
    }
}