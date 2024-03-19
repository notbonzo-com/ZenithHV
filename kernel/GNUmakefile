# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

ANSI_GREEN := \033[0;32m
ANSI_RESET := \033[0m

override KERNEL := kernel.bin

# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef


override DEFAULT_CC := ../../toolchain/x86_64-elf/bin/x86_64-elf-gcc
$(eval $(call DEFAULT_VAR,CC,$(DEFAULT_CC)))

override DEFAULT_LD := ../../toolchain/x86_64-elf/bin/x86_64-elf-ld
$(eval $(call DEFAULT_VAR,LD,$(DEFAULT_LD)))

override DEFAULT_AS := ../../toolchain/x86_64-elf/bin/x86_64-elf-as
$(eval $(call DEFAULT_VAR,AS,$(DEFAULT_AS)))

override DEFAULT_CFLAGS := -g -O2 -pipe
$(eval $(call DEFAULT_VAR,CFLAGS,$(DEFAULT_CFLAGS)))

# User controllable C preprocessor flags. We set none by default.
override DEFAULT_CPPFLAGS :=
$(eval $(call DEFAULT_VAR,CPPFLAGS,$(DEFAULT_CPPFLAGS)))

override DEFAULT_CXXFLAGS := -g -O2 -pipe -std=c++17
$(eval $(call DEFAULT_VAR,CXXFLAGS,$(DEFAULT_CXXFLAGS)))

override DEFAULT_CXX := ../../toolchain/x86_64-elf/bin/x86_64-elf-g++
$(eval $(call DEFAULT_VAR,CXX,$(DEFAULT_CXX)))

override DEFAULT_NASMFLAGS := -F dwarf -g
$(eval $(call DEFAULT_VAR,NASMFLAGS,$(DEFAULT_NASMFLAGS)))

override DEFAULT_LDFLAGS :=
$(eval $(call DEFAULT_VAR,LDFLAGS,$(DEFAULT_LDFLAGS)))

override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fPIE \
    -m64 \
    -march=x86-64 \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
    -I ./inc/x86 \
    -I ./ \
    -I ./inc \
    -I ./cfg \
    -Wno-trigraphs

override CPPFLAGS := \
    -I . \
    -I ./cfg \
    $(CPPFLAGS) \
    -MMD \
    -MP

# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
    -m elf_x86_64 \
    -nostdlib \
    -static \
    -pie \
    --no-dynamic-linker \
    -z text \
    -z max-page-size=0x1000 \
    -T cfg/linker.ld

# Internal nasm flags that should not be changed by the user.
override NASMFLAGS += \
    -Wall \
    -f elf64

override CFILES := $(shell find -L * -type f -name '*.c')
override ASFILES := $(shell find -L * -type f -name '*.S')
override NASMFILES := $(shell find -L * -type f -name '*.asm')
override ASFILES := $(shell find -L * -type f -name '*.S')
override OBJ := $(addprefix ../build/obj/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))
override HEADER_DEPS := $(addprefix ../build/obj/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Default target.
.PHONY: all
all: ../build/$(KERNEL)

limine.h:
	curl -Lo $@ https://github.com/limine-bootloader/limine/raw/trunk/limine.h

# Link rules for the final kernel executable.
../build/$(KERNEL): GNUmakefile cfg/linker.ld $(OBJ)
	mkdir -p "$$(dirname $@)"
	@echo -e "$(ANSI_GREEN)LINKING$(ANSI_RESET) $@"
	@$(LD) $(OBJ) $(LDFLAGS) -o $@

# Include header dependencies.
-include $(HEADER_DEPS)

# Compilation rules for *.c files.
../build/obj/%.c.o: %.c GNUmakefile cfg/limine.h
	mkdir -p "$$(dirname $@)"
	@echo -e "$(ANSI_GREEN)COMPILING$(ANSI_RESET) $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
../build/obj/%.S.o: %.S GNUmakefile
	mkdir -p "$$(dirname $@)"
	@echo -e "$(ANSI_GREEN)COMPILING$(ANSI_RESET) $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.asm (nasm) files.
../build/obj/%.asm.o: %.asm GNUmakefile
	mkdir -p "$$(dirname $@)"
	@echo -e "$(ANSI_GREEN)COMPILING$(ANSI_RESET) $<"
	@nasm $(NASMFLAGS) $< -o $@



# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf ../build/obj ../build/$(KERNEL)

.PHONY: distclean
distclean: clean
	rm -f limine.h
