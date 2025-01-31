# config.mk

export ANSI_GREEN := \033[0;32m
export ANSI_RESET := \033[0m

export DEFAULT_CC := $(HOME)/toolchain/x86_64-elf/bin/x86_64-elf-gcc
export DEFAULT_LD := $(HOME)/toolchain/x86_64-elf/bin/x86_64-elf-ld
export DEFAULT_AS := $(HOME)/toolchain/x86_64-elf/bin/x86_64-elf-as
export DEFAULT_AR := $(HOME)/toolchain/x86_64-elf/bin/x86_64-elf-ar
export DEFAULT_CXX := $(HOME)/toolchain/x86_64-elf/bin/x86_64-elf-g++
export OBJDUMP := $(HOME)/toolchain/x86_64-elf/bin/x86_64-elf-objdump
export OBJCOPY := $(HOME)/toolchain/x86_64-elf/bin/x86_64-elf-objcopy
export NM := $(HOME)/toolchain/x86_64-elf/bin/x86_64-elf-nm

export DEFAULT_CFLAGS := -std=c23
export DEFAULT_CXXFLAGS := -std=c++23

export DEFAULT_CCFLAGS := -g -O2 -pipe -Wall -Wextra -Werror -pedantic

export DEFAULT_NASMFLAGS := -Wall -g
export DEFAULT_LDFLAGS := -g -O2

export KERNEL_CFLAGS := $(DEFAULT_CFLAGS) $(DEFAULT_CCFLAGS) -fPIE -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -ffreestanding \
	-fno-omit-frame-pointer -fno-lto -nostdlib -nostartfiles \
	-I ./std -I .
export KERNEL_CXXFLAGS := $(DEFAULT_CXXFLAGS) $(DEFAULT_CCFLAGS) -fPIE -m64 -march=x86-64 -mno-80387 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -ffreestanding \
	-Wno-delete-non-virtual-dtor -nostdinc++ -fno-omit-frame-pointer -fno-use-cxa-atexit -fno-exceptions -fno-rtti -fstack-protector-strong \
	-I ./std -I .
export KERNEL_LDFLAGS := $(DEFAULT_LDFLAGS) -ffreestanding -nostdlib -nostartfiles -T linker.ld -static -lgcc
export KERNEL_NASMFLAGS := $(DEFAULT_NASMFLAGS) -f elf64