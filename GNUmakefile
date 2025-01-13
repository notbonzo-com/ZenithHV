override MAKEFLAGS += -rRs
CORES := $(shell nproc)
BUILD_DIR := $(abspath ./build)
EXTERNAL_DIR := $(abspath ./external)
LIMINE_DIR := $(abspath $(EXTERNAL_DIR)/limine)

include config.mk

all: kernel ramfs disk run

kernel:
	@make -C kernel/ BUILD_DIR=$(BUILD_DIR)

ramfs:
	@make -C ramfs/ BUILD_DIR=$(BUILD_DIR)

disk:
	@dd if=/dev/zero bs=1M count=0 seek=64 of=$(BUILD_DIR)/image.hdd
	@sgdisk $(BUILD_DIR)/image.hdd -n 1:2048 -t 1:ef00

	@$(LIMINE_DIR)/limine bios-install $(BUILD_DIR)/image.hdd
	@mformat -i $(BUILD_DIR)/image.hdd@@1M
	@mmd -i $(BUILD_DIR)/image.hdd@@1M ::/EFI ::/EFI/BOOT

	@mcopy -i $(BUILD_DIR)/image.hdd@@1M $(BUILD_DIR)/kernel.bin kernel/cfg/limine.cfg $(LIMINE_DIR)/limine-bios.sys ::/
	@mcopy -i $(BUILD_DIR)/image.hdd@@1M $(LIMINE_DIR)/BOOTX64.EFI ::/EFI/BOOT
	@mcopy -i $(BUILD_DIR)/image.hdd@@1M $(LIMINE_DIR)/BOOTIA32.EFI ::/EFI/BOOT

	@mcopy -i $(BUILD_DIR)/image.hdd@@1M $(BUILD_DIR)/initramfs.img ::/

limine:
	@git clone https://github.com/limine-bootloader/limine.git --branch=v7.x-binary --depth=1 $(LIMINE_DIR)
	@make -C $(LIMINE_DIR)
reinstall-limine:
	@rm -rf $(LIMINE_DIR)/
	@make limine

run:
	@clear
	@qemu-system-x86_64 -drive format=raw,file=$(BUILD_DIR)/image.hdd \
			-m 4G -enable-kvm -cpu host,+svm -smp cores=$(CORES) -M q35 \
			-debugcon stdio \
			--no-reboot \
			-serial file:$(BUILD_DIR)/serial_output.txt \
			-d int -M smm=off \
			-device pci-bridge,chassis_nr=3,id=b2 \
			-D $(BUILD_DIR)/qemu_log.txt -d guest_errors,cpu_reset

clean:
	@clear
	@make -C kernel/ clean
	@rm -rf $(BUILD_DIR)/image.hdd $(BUILD_DIR)/image.iso iso_root/ $(BUILD_DIR)/initramfs.img
	@rm -rf $(BUILD_DIR)/serial_output.txt $(BUILD_DIR)/monitor_output.txt $(BUILD_DIR)/qemu_log.txt

reset:
	@make clean
	@clear
	@make

.PHONY: all kernel disk ramfs run clean reset reinstall-limine
