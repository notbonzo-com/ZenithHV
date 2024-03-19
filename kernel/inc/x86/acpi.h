#pragma once

#define MADT_ENTRY_PROCESSOR_LAPIC 0x0
#define MADT_ENTRY_IO_APIC 0x1
#define MADT_ENTRY_INTERRUPT_SOURCE_OVERRIDE 0x2
#define MADT_ENTRY_NON_MASKABLE_ADDRESS_OVERRIDE 0x3
#define MADT_ENTRY_LAPIC_NMI 0x4
#define MADT_ENTRY_LAPIC_ADDRESS_OVERRIDE 0x5
#define MADT_ENTRY_IO_SAPIC 0x6
#define MADT_ENTRY_LSAPIC 0x7
#define MADT_ENTRY_PLATFORM_INTERRUPT_SOURCES 0x8
#define MADT_ENTRY_PROCESSOR_LOCAL_X2APIC 0x9
#define MADT_ENTRY_LOCAL_X2APIC_NMI 0xA
#define MADT_ENTRY_GIC_CPU_INTERFACE 0xB
#define MADT_ENTRY_GIC_DISTRIBUTOR 0xC
#define MADT_ENTRY_GIC_MSI_FRAME 0xD
#define MADT_ENTRY_GIC_REDISTRIBUTOR 0xE
#define MADT_ENTRY_GIC_INTERRUPT_TRANSLATION_SERVICE 0xF
#define MADT_ENTRY_MP_WAKEUP 0x10

struct Rsdp {
    char signature[8];
    uint8_t checksum;
    char oemId[6];
    uint8_t revision;
    uint32_t rsdtAddress;

    uint32_t length;
    uint64_t xsdtAddress;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
} __attribute__ ((packed));

struct SdtHeader {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oemId[6];
    char oemTableId[8];
    uint32_t oemRevision;
    uint32_t creatorId;
    uint32_t creatorRevision;
};

struct Rsdt {
  struct SdtHeader header;
  uint8_t ptr[];
};

struct Gas {
    uint8_t addressSpace;
    uint8_t bitWidth;
    uint8_t bitOffset;
    uint8_t accessSize;
    uint64_t address;
} __attribute__((packed));

struct fadt {
    struct SdtHeader header;
    uint32_t firmwareCtrl;
    uint32_t dsdt;
    uint8_t reserved0;
    uint8_t preferredPowerManagementProfile;
    uint16_t sciInterrupt;
    uint32_t smiCommandPort;
    uint8_t acpiEnable;
    uint8_t acpiDisable;
    uint8_t s4biosReq;
    uint8_t pstateControl;
    uint32_t pm1aEventBlock;
    uint32_t pm1bEventBlock;
    uint32_t pm1aControlBlock;
    uint32_t pm1bControlBlock;
    uint32_t pm2ControlBlock;
    uint32_t pmTimerBlock;
    uint32_t gpe0Block;
    uint32_t gpe1Block;
    uint8_t pm1EventLength;
    uint8_t pm1ControlLength;
    uint8_t pm12ControlLength;
    uint8_t pmTimerLength;
    uint8_t gpe0Length;
    uint8_t gpe1Length;
    uint8_t gpe1Base;
    uint8_t cSateControl;
    uint16_t worstC2Latency;
    uint16_t worstC3Latency;
    uint16_t flushSize;
    uint16_t flushStride;
    uint8_t dutyOffset;
    uint8_t dutyWidth;
    uint8_t dayAlarm;
    uint8_t monthAlarm;
    uint8_t century;
    uint16_t bootArchFlags;
    uint8_t reserved1;
    uint32_t flags;
    struct Gas resetReg;
    uint8_t resetValue;
    uint8_t reserved2[3];
    uint64_t xFirmwareControl;
    uint64_t xDsdt;
    struct Gas xPm1aEventBlock;
    struct Gas xPm1bEventBlock;
    struct Gas xPm1aControlBlock;
    struct Gas xPm1bControlBlock;
    struct Gas xPm2ControlBlock;
    struct Gas xPmTimerBlock;
    struct Gas xGpe0Block;
    struct Gas xGpe1Block;
};

struct Madt {
    struct SdtHeader header;
    uint32_t lapicAddressPhys;
    uint32_t flags;
    uint8_t entries[];
};

struct MadtHeader {
    uint8_t lcstId;
    uint8_t length;
} __attribute__((packed));

struct Lapic {
    struct MadtHeader header;
    uint8_t acpiProcessorUid;
    uint8_t apicId;
    uint32_t flags;
} __attribute__((packed));

struct Ioapic {
    struct MadtHeader header;
    uint8_t ioApicId;
    uint8_t reserved;
    uint32_t ioApicAddress;
    uint32_t globalSystemInterruptBase;
} __attribute__((packed));

struct Iso {
    struct MadtHeader header;
    uint8_t busIsa;
    uint8_t sourceIrq;
    uint32_t globalSystemInterrupt;
    uint16_t flags;
} __attribute__((packed));

void initAcpi(void);
void *sdtFind(const char signature[static 4]);
void parseMADT(struct Madt *madt);

extern struct Rsdp *RsdpPtr;
extern struct Rsdt *RsdtPtr;

extern struct Fadt *fadtPtr;
extern struct Madt *madtPtr;

extern struct limine_rsdp_request limineRsdpRequest;

extern struct Ioapic *ioapics;
extern struct Lapic *lapics;
extern struct Iso *isos;

extern size_t ioapicCount;
extern size_t lapicCount;
extern size_t isoCount;