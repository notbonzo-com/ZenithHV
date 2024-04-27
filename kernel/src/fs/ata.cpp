#include <fs/ata.h>
#include <kalloc.h>
#include <x86/io.h>
#include <kprintf.h>
#include <x86/idt.h>
#include <x86/smp.h>

#define ATA_PRIMARY_IO 0x1F0
#define ATA_SECONDARY_IO 0x170

#define ATA_PRIMARY_DCR_AS 0x3F6
#define ATA_SECONDARY_DCR_AS 0x376

#define ATA_PRIMARY_IRQ 14
#define ATA_SECONDARY_IRQ 15

uint8_t ata_pm = 0; /* Primary master exists? */
uint8_t ata_ps = 0; /* Primary Slave exists? */
uint8_t ata_sm = 0; /* Secondary master exists? */
uint8_t ata_ss = 0; /* Secondary slave exists? */

uint8_t *ide_buf = 0;

extern "C" {

void ide_select_drive(uint8_t bus, uint8_t i)
{
	if(bus == ATA_PRIMARY)
		if(i == ATA_MASTER)
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xB0);
	else
		if(i == ATA_MASTER)
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xB0);
}


void ide_primary_irq()
{}

void ide_secondary_irq()
{}

klock identify_lock;

uint8_t ide_identify(uint8_t bus, uint8_t drive)
{
	uint16_t io = 0;
	ide_select_drive(bus, drive);
	if(bus == ATA_PRIMARY) io = ATA_PRIMARY_IO;
	else io = ATA_SECONDARY_IO;
	outb(io + ATA_REG_SECCOUNT0, 0);
	outb(io + ATA_REG_LBA0, 0);
	outb(io + ATA_REG_LBA1, 0);
	outb(io + ATA_REG_LBA2, 0);
	outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
	debugf("Sent IDENTIFY command to %s", bus==ATA_PRIMARY?"Primary":"Secondary");
	uint8_t status = inb(io + ATA_REG_STATUS);
	if(status)
	{
		while((inb(io + ATA_REG_STATUS) & ATA_SR_BSY) != 0) ;
pm_stat_read:		status = inb(io + ATA_REG_STATUS);
		if(status & ATA_SR_ERR)
		{
			debugf("%s%s has ERR set. Disabled.", bus==ATA_PRIMARY?"Primary":"Secondary", drive==ATA_PRIMARY?" master":" slave");
			return 0;
		}
		while(!(status & ATA_SR_DRQ)) goto pm_stat_read;
		debugf("%s%s is online.", bus==ATA_PRIMARY?"Primary":"Secondary", drive==ATA_PRIMARY?" master":" slave");
		identify_lock.acquire();
		for(int i = 0; i<256; i++)
		{
			*(uint16_t *)(ide_buf + i*2) = inw(io + ATA_REG_DATA);
		}
		identify_lock.release();
	}
    return 1;
}

void ide_400ns_delay(uint16_t io)
{
	for(int i = 0;i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);
}

void ide_poll(uint16_t io)
{
	
	for(int i=0; i< 4; i++)
		inb(io + ATA_REG_ALTSTATUS);

retry:;
	uint8_t status = inb(io + ATA_REG_STATUS);
	if(status & ATA_SR_BSY) goto retry;
retry2:	status = inb(io + ATA_REG_STATUS);
	if(status & ATA_SR_ERR)
	{
		// panic("ERR set, device failure!");
        debugf("ERROR: Device failed");
        kpanic();
	}
	if(!(status & ATA_SR_DRQ)) goto retry2;
	return;
}

klock ata_read_lock;

uint8_t ata_read_one(uint8_t *buf, uint32_t lba)
{
	uint16_t io = ATA_PRIMARY_IO;
    uint16_t drive = ATA_MASTER;
	uint8_t cmd = (drive==ATA_MASTER?0xE0:0xF0);
	// uint8_t slavebit = (drive == ATA_MASTER?0x00:0x01);
    // debugf("Slave bit is %d", slavebit);
	outb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	outb(io + 1, 0x00);
	outb(io + ATA_REG_SECCOUNT0, 1);
	outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
	outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
	ide_poll(io);

	ata_read_lock.acquire();
	for(int i = 0; i < 256; i++)
	{
		uint16_t data = inw(io + ATA_REG_DATA);
		*(uint16_t *)(buf + i * 2) = data;
	}
	ide_400ns_delay(io);
	ata_read_lock.release();
	return 1;
}

void ata_read(uint8_t *buf, uint32_t lba, uint32_t numsects)
{
	for(uint32_t i = 0; i < numsects; i++)
	{
		ata_read_one(buf, lba + i);
		buf += 512;
	}
}

void ata_probe()
{
	if (ide_identify(ATA_PRIMARY, ATA_MASTER))
	{
		ata_pm = 1;
		char *str = (char *)kmalloc(40);
		for(int i = 0; i < 40; i += 2)
		{
			str[i] = ide_buf[ATA_IDENT_MODEL + i + 1];
			str[i + 1] = ide_buf[ATA_IDENT_MODEL + i];
		}
        debugf("Device: %s", str);
        debugf("Device drive: %d", (ATA_PRIMARY << 1) | ATA_MASTER);
        kfree(str);
	}
	ide_identify(ATA_PRIMARY, ATA_SLAVE);
}

void ata_init()
{
	debugf("Checking for ATA drives");
	ide_buf = (uint8_t *)kmalloc(512);
    IDT_SetVector(ATA_PRIMARY_IRQ, (uintptr_t)ide_primary_irq);
    IDT_SetVector(ATA_SECONDARY_IRQ, (uintptr_t)ide_secondary_irq);
	ata_probe();
}

}