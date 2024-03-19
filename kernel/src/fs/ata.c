#include <fs/ata.h>
#include <stdint.h>
#include <x86/io.h>
#include <x86/int.h>

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <debug.h>

uint8_t ata_pm = 0; /* Primary master exists? */
uint8_t ata_ps = 0; /* Primary Slave exists? */
uint8_t ata_sm = 0; /* Secondary master exists? */
uint8_t ata_ss = 0; /* Secondary slave exists? */

uint8_t *ide_buf = 0;

void ide_select_drive(uint8_t bus, uint8_t i)
{
	if (bus == ATA_PRIMARY)
		if (i == ATA_MASTER)
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xB0);
	else
		if (i == ATA_MASTER)
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		else outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xB0);
}

void ide_primary_irq()
{
	sendEoi(); // Inline
}

void ide_secondary_irq()
{
	sendEoi(); // Inline
}

void ide400nsDelay(uint16_t io)
{
	for(int i = 0; i < 4; i++)
		inb(io + ATA_REG_ALTSTATUS);
}

void idePoll(uint16_t io)
{
	ide400nsDelay(io);
    log_debug("ATA", "Polling 400ns completion\n");
retry:;
	uint8_t status = inb(io + ATA_REG_STATUS);
    log_debug("ATA", "Status: %x\n", status);
	if (status & ATA_SR_BSY)
        goto retry;
retry2:
    status = inb(io + ATA_REG_STATUS);
	if (status & ATA_SR_ERR)
	{
        log_crit("ATA", "Error occured while polling\n");        
        return;
	}
	if (!(status & ATA_SR_DRQ))
        goto retry2;
	return;
}

void WaitBSY(uint8_t io)
{
    while ((inb(io + ATA_REG_STATUS) & ATA_SR_BSY) != 0);
}

uint8_t ATAIdentify(uint8_t bus, uint8_t drive)
{
	uint16_t io = 0;
	ide_select_drive(bus, drive);
	if (bus == ATA_PRIMARY)
        io = ATA_PRIMARY_IO;
	else
        io = ATA_SECONDARY_IO;
	outb(io + ATA_REG_SECCOUNT0, 0);
	outb(io + ATA_REG_LBA0, 0);
	outb(io + ATA_REG_LBA1, 0);
	outb(io + ATA_REG_LBA2, 0);
	outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

    log_debug("ATA", "Issued IDENTIFY to 0x%x\n", io + ATA_REG_COMMAND);

	uint8_t status = inb(io + ATA_REG_STATUS);
	if(status)
	{
        WaitBSY(io);
pm_stat_read:
        status = inb(io + ATA_REG_STATUS);
		if (status & ATA_SR_ERR)
		{
            log_err("ATA", "%s%s has ERR set. Disabled.\n", bus==ATA_PRIMARY?"Primary":"Secondary", drive==ATA_PRIMARY?" master":" slave");
			return 1;
		}
		while(!(status & ATA_SR_DRQ))
            goto pm_stat_read;
        log_debug("ATA", "%s%s is online.\n", bus==ATA_PRIMARY?"Primary":"Secondary", drive==ATA_PRIMARY?" master":" slave");
		for (int i = 0; i<256; i++)
		{
			*(uint16_t *)(ide_buf + i*2) = inw(io + ATA_REG_DATA);
		}
	}
	else
	{
		log_err("ATA", "%s%s is offline.\n", bus==ATA_PRIMARY?"Primary":"Secondary", drive==ATA_PRIMARY?" master":" slave");
		return 1;
	}
	return 0;
}

uint8_t AtaReadOne(uint8_t *buf, uint32_t lba, uint8_t drive)
{
	uint16_t io = 0;
	switch(drive)
	{
		case (ATA_PRIMARY << 1 | ATA_MASTER):
			io = ATA_PRIMARY_IO;
			drive = ATA_MASTER;
			break;
		case (ATA_PRIMARY << 1 | ATA_SLAVE):
			io = ATA_PRIMARY_IO;
			drive = ATA_SLAVE;
			break;
		case (ATA_SECONDARY << 1 | ATA_MASTER):
			io = ATA_SECONDARY_IO;
			drive = ATA_MASTER;
			break;
		case (ATA_SECONDARY << 1 | ATA_SLAVE):
			io = ATA_SECONDARY_IO;
			drive = ATA_SLAVE;
			break;
		default:
			log_err("ATA", "Invalid drive specified\n");
			return 1;
	}
    log_debug("ATA",  "io=0x%x %s\n", io, drive==ATA_MASTER?"Master":"Slave");
	uint8_t cmd = (drive==ATA_MASTER?0xE0:0xF0);

	outb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	outb(io + 1, 0x00);
	outb(io + ATA_REG_SECCOUNT0, 1);
	outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
	outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	log_debug("ATA",  "issued 0x%x to 0x%x\n", (uint8_t)((lba) >> 8), io + ATA_REG_LBA1);
	outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
    log_debug("ATA",  "issued 0x%x to 0x%x\n", ATA_CMD_READ_PIO, io + ATA_REG_COMMAND);
	outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    log_debug("ATA",  "issued 0x%x to 0x%x\n", ATA_CMD_READ_PIO, io + ATA_REG_COMMAND);

	log_debug("ATA",  "Polling\n");
	idePoll(io);

	for (int i = 0; i < 256; i++)
	{
		uint16_t data = inw(io + ATA_REG_DATA);
		*(uint16_t *)(buf + i * 2) = data;
	}
	ide400nsDelay(io);

	return 0;
}

uint8_t AtaWriteOne(uint8_t buf[512], uint32_t lba, uint8_t drive)
{
	uint16_t io = 0;
	switch(drive)
	{
		case (ATA_PRIMARY << 1 | ATA_MASTER):
			io = ATA_PRIMARY_IO;
			drive = ATA_MASTER;
			break;
		case (ATA_PRIMARY << 1 | ATA_SLAVE):
			io = ATA_PRIMARY_IO;
			drive = ATA_SLAVE;
			break;
		case (ATA_SECONDARY << 1 | ATA_MASTER):
			io = ATA_SECONDARY_IO;
			drive = ATA_MASTER;
			break;
		case (ATA_SECONDARY << 1 | ATA_SLAVE):
			io = ATA_SECONDARY_IO;
			drive = ATA_SLAVE;
			break;
		default:
			log_err("ATA", "Invalid drive specified\n");
			return 1;
	}
	uint8_t cmd = (drive==ATA_MASTER?0xE0:0xF0);
	uint8_t slavebit = (drive == ATA_MASTER?0x00:0x01);
	log_debug("ATA", "io=0x%x %s\n", io, drive==ATA_MASTER?"Master":"Slave");
	log_debug("ATA", "cmd=0x%x\n", cmd);
	log_debug("ATA", "slavebit=0x%x\n", slavebit);

	outb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
	outb(io + 1, 0x00);
	outb(io + ATA_REG_SECCOUNT0, 1);
	outb(io + ATA_REG_LBA0, (uint8_t)((lba)));
	outb(io + ATA_REG_LBA1, (uint8_t)((lba) >> 8));
	outb(io + ATA_REG_LBA2, (uint8_t)((lba) >> 16));
	outb(io + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

	idePoll(io);

	for(int i = 0; i < 256; i++)
	{
		uint16_t data = *(uint16_t *)(buf + i * 2);
		outw(io + ATA_REG_DATA, data);
	}
	ide400nsDelay(io);
	return 0;
}

void ATAWrite(uint8_t *buf, uint32_t lba, uint32_t numsects, uint8_t drive)
{
	for(uint32_t i = 0; i < numsects; i++)
	{
		AtaWriteOne(buf, lba + i, drive);
		buf += 512;
	}
}

void ATARead(uint8_t *buf, uint32_t lba, uint32_t numsects, uint8_t drive)
{
	for (uint32_t i = 0; i < numsects; i++)
	{
		AtaReadOne(buf, lba + i, drive);
		buf += 512;
	}
}