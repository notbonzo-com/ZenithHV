#pragma once

#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <vector>
#include <stdint.h>
#include <util>

constexpr uint8_t MADT_ENTRY_PROCESSOR_LAPIC = 0x0;
constexpr uint8_t MADT_ENTRY_IO_APIC = 0x1;
constexpr uint8_t MADT_ENTRY_INTERRUPT_SOURCE_OVERRIDE = 0x2;
constexpr uint8_t MADT_ENTRY_NON_MASKABLE_ADDRESS_OVERRIDE = 0x3;
constexpr uint8_t MADT_ENTRY_LAPIC_NMI = 0x4;
constexpr uint8_t MADT_ENTRY_LAPIC_ADDRESS_OVERRIDE = 0x5;
constexpr uint8_t MADT_ENTRY_IO_SAPIC = 0x6;
constexpr uint8_t MADT_ENTRY_LSAPIC = 0x7;
constexpr uint8_t MADT_ENTRY_PLATFORM_INTERRUPT_SOURCES = 0x8;
constexpr uint8_t MADT_ENTRY_PROCESSOR_LOCAL_X2APIC = 0x9;
constexpr uint8_t MADT_ENTRY_LOCAL_X2APIC_NMI = 0xA;
constexpr uint8_t MADT_ENTRY_GIC_CPU_INTERFACE = 0xB;
constexpr uint8_t MADT_ENTRY_GIC_DISTRIBUTOR = 0xC;
constexpr uint8_t MADT_ENTRY_GIC_MSI_FRAME = 0xD;
constexpr uint8_t MADT_ENTRY_GIC_REDISTRIBUTOR = 0xE;
constexpr uint8_t MADT_ENTRY_GIC_INTERRUPT_TRANSLATION_SERVICE = 0xF;
constexpr uint8_t MADT_ENTRY_MP_WAKEUP = 0x10;

namespace acpi {

// ACPI RSDP Structure
struct __pack RSDP {
    char signature[8];
    uint8_t checksum;
    char oemid[6];
    uint8_t revision;
    uint32_t rsdt_address;
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
    uint8_t reserved[3];
};

// ACPI SDT Header Structure
struct SDTHeader {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
};

// ACPI RSDT Structure
struct RSDT {
    SDTHeader header;
};

// Generic Address Structure (GAS)
struct __pack GAS {
    uint8_t address_space;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t access_size;
    uint64_t address;
};

// ACPI FADT Structure
struct FADT {
    SDTHeader header;
    uint32_t firmware_ctrl;
    uint32_t dsdt;

    uint8_t reserved_0;
    uint8_t preferred_power_management_profile;
    uint16_t sci_interrupt;
    uint32_t smi_command_port;
    uint8_t acpi_enable;
    uint8_t acpi_disable;
    uint8_t s4bios_req;
    uint8_t pstate_control;
    uint32_t pm1a_event_block;
    uint32_t pm1b_event_block;
    uint32_t pm1a_control_block;
    uint32_t pm1b_control_block;
    uint32_t pm2_control_block;
    uint32_t pm_timer_block;
    uint32_t gpe0_block;
    uint32_t gpe1_block;
    uint8_t pm1_event_length;
    uint8_t pm1_control_length;
    uint8_t pm12_control_length;
    uint8_t pm_timer_length;
    uint8_t gpe0_length;
    uint8_t gpe1_length;
    uint8_t gpe1_base;
    uint8_t c_state_control;
    uint16_t worst_c2_latency;
    uint16_t worst_c3_latency;
    uint16_t flush_size;
    uint16_t flush_stride;
    uint8_t duty_offset;
    uint8_t duty_width;
    uint8_t day_alarm;
    uint8_t month_alarm;
    uint8_t century;

    uint16_t boot_arch_flags;
    uint8_t reserved_1;
    uint32_t flags;

    GAS reset_reg;
    uint8_t reset_value;
    uint8_t reserved_2[3];

    uint64_t x_firmware_control;
    uint64_t x_dsdt;

    GAS x_pm1a_event_block;
    GAS x_pm1b_event_block;
    GAS x_pm1a_control_block;
    GAS x_pm1b_control_block;
    GAS x_pm2_control_block;
    GAS x_pm_timer_block;
    GAS x_gpe0_block;
    GAS x_gpe1_block;
};

// ACPI MADT Structure
struct MADT {
    SDTHeader header;
    uint32_t lapic_address_phys;
    uint32_t flags;
    uint8_t entries[];
};

// ACPI MCFG Entry Structure
struct __pack MCFGEntry {
    uint64_t base;
    uint16_t segment;
    uint8_t host_start;
    uint8_t host_end;
    uint32_t reserved;
};

// ACPI MCFG Structure
struct __pack MCFG {
    SDTHeader header;
    uint64_t reserved;
    MCFGEntry entries[];
};

// ACPI MADT Header Structure
struct __pack MADTHeader {
    uint8_t type;
    uint8_t length;
};

// ACPI LAPIC Structure
struct __pack LAPIC {
    MADTHeader header;
    uint8_t acpi_processor_uid;
    uint8_t apic_id;
    uint32_t flags;
};

// ACPI IOAPIC Structure
struct __pack IOAPIC {
    MADTHeader header;
    uint8_t io_apic_id;
    uint8_t reserved;
    uint32_t io_apic_address;
    uint32_t global_system_interrupt_base;
};
// ACPI ISO Structure
struct __pack ISO {
    MADTHeader header;
    uint8_t bus_isa;
    uint8_t source_irq;
    uint32_t global_system_interrupt;
    uint16_t flags;
};

// Function Prototypes
void parse();
void* get_sdt(const char signature[4]);
void parse_madt(volatile MADT* madt);

// External Variables
extern struct limine_rsdp_request rsdp_request;
extern std::vector<IOAPIC*> ioapics;
extern std::vector<LAPIC*> lapics;
extern std::vector<ISO*> isos;

extern volatile FADT* fadt_ptr;
extern volatile MADT* madt_ptr;
extern volatile MCFG* mcfg_ptr;

} // namespace acpi
