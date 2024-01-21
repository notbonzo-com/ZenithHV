.PHONY: all kernel floppy disk limine run run_disk clean reset reinstall-limine
all: kernel disk run_disk

kernel:
	@make -C kernel/

floppy:
	@cp -v build/kernel.bin kernel/limine.cfg limine/limine-bios.sys \
      limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/
	@mkdir -p iso_root/EFI/BOOT
	@cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	@cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/

	@xorriso -as mkisofs -b limine-bios-cd.bin \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        --efi-boot limine-uefi-cd.bin \
        -efi-boot-part --efi-boot-image --protective-msdos-label \
        iso_root -o build/image.iso

	@./limine/limine bios-install build/image.iso

disk:
	@dd if=/dev/zero bs=1M count=0 seek=64 of=build/image.hdd
	@sgdisk build/image.hdd -n 1:2048 -t 1:ef00

	@./limine/limine bios-install build/image.hdd
	@mformat -i build/image.hdd@@1M
	@mmd -i build/image.hdd@@1M ::/EFI ::/EFI/BOOT

	@mcopy -i build/image.hdd@@1M build/kernel.bin kernel/limine.cfg limine/limine-bios.sys ::/
	@mcopy -i build/image.hdd@@1M limine/BOOTX64.EFI ::/EFI/BOOT
	@mcopy -i build/image.hdd@@1M limine/BOOTIA32.EFI ::/EFI/BOOT

limine:
	@git clone https://github.com/limine-bootloader/limine.git --branch=binary --depth=1
	@make -C limine
	@mkdir -p iso_root

run:
	@clear
	@qemu-system-x86_64 -cdrom build/image.iso -m 512M -smp 2 -serial stdio -debugcon stdio

run_disk:
	@clear
	@qemu-system-x86_64 -drive format=raw,file=build/image.hdd -m 512M -smp 2 -serial file:build/serial_output.txt -debugcon stdio

clean:
	@make -C kernel/ clean
	@rm -rf build/image.hdd build/image.iso iso_root/
	@rm -rf build/serial_output.txt

reset:
	@make clean
	@clear
	@make

reinstall-limine:
	@rm -rf limine/
	@make limine