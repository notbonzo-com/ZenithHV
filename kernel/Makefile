BUILD_DIR ?= ../build

override KERNEL := kernel.bin

override CFILES := $(shell find -L * -type f -name '*.c')
override ASFILES := $(shell find -L * -type f -name '*.S')
override NASMFILES := $(shell find -L * -type f -name '*.asm')
override ASFILES := $(shell find -L * -type f -name '*.S')
override HEADER_DEPS := $(addprefix $(BUILD_DIR)/kernel/obj/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))
override CXXFILES := $(shell find -L * -type f -name '*.cc')
override OBJ := $(addprefix $(BUILD_DIR)/kernel/obj/,$(CFILES:.c=.c.o) $(CXXFILES:.cc=.cc.o) $(ASFILES:.S=.S.o) $(NASMFILES:.asm=.asm.o))

override CC := $(DEFAULT_CC)
override CXX := $(DEFAULT_CXX)
override LD := $(DEFAULT_LD)

.PHONY: all
all: $(BUILD_DIR)/$(KERNEL)

std/limine.h:
	@curl -Lo $@ https://github.com/limine-bootloader/limine/raw/trunk/limine.h std/limine.h

$(BUILD_DIR)/$(KERNEL): Makefile linker.ld $(OBJ)
	@mkdir -p "$$(dirname $@)"
	@echo "$(ANSI_GREEN)LINKING$(ANSI_RESET) $@"
	@$(CC) $(OBJ) $(KERNEL_LDFLAGS) -o $@

-include $(HEADER_DEPS)

$(BUILD_DIR)/kernel/obj/%.c.o: %.c Makefile std/limine.h
	@mkdir -p "$$(dirname $@)"
	@echo "$(ANSI_GREEN)COMPILING$(ANSI_RESET) $<"
	@$(CC) $(KERNEL_CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/obj/%.cc.o: %.cc Makefile std/limine.h
	@mkdir -p "$$(dirname $@)"
	@echo "$(ANSI_GREEN)COMPILING$(ANSI_RESET) $<"
	@$(CXX) $(KERNEL_CXXFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/obj/%.S.o: %.S Makefile
	@mkdir -p "$$(dirname $@)"
	@echo "$(ANSI_GREEN)ASSEMBLING$(ANSI_RESET) $<"
	@$(CC) $(KERNEL_CFLAGS) $(KERNEL_CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/obj/%.asm.o: %.asm Makefile
	@mkdir -p "$$(dirname $@)"
	@echo "$(ANSI_GREEN)ASSEMBLING$(ANSI_RESET) $<"
	@nasm $(NASMFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)/kernel $(BUILD_DIR)/$(KERNEL)

.PHONY: distclean
distclean: clean
	rm -f limine.h