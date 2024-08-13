BUILD_DIR ?= ../../build

.PHONY: all clean

override LIBK := $(BUILD_DIR)/libk.a

BINARIES = libk

all: $(BINARIES)

override CFILES := $(shell find -L * -type f -name '*.c')
override ASFILES := $(shell find -L * -type f -name '*.S')
override NASMFILES := $(shell find -L * -type f -name '*.asm')
override CXXFILES := $(shell find -L * -type f -name '*.cpp')
override OBJ := $(addprefix $(BUILD_DIR)/lib/obj/,$(CXXFILES:%.cpp=%.cpp.o) $(CFILES:%.c=%.c.o) $(ASFILES:%.S=%.S.o) $(NASMFILES:%.asm=%.asm.o))

override CC := $(DEFAULT_CC)
override CXX := $(DEFAULT_CXX)
override LD := $(DEFAULT_LD)
override AR := $(DEFAULT_AR)

override CFLAGS := -Wall -Werror -nostdlib -I$(SYSROOT)/usr/include
override CXXFLAGS := -Wall -Werror -nostdlib -I$(SYSROOT)/usr/include

libk: $(OBJ)
	$(AR) rcs $(LIBK) $(OBJ)

$(BUILD_DIR)/lib/obj/%.cpp.o: %.cpp
	@echo "Compiling $< to $@"
	@mkdir -p "$(dir $@)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/obj/%.c.o: %.c
	@echo "Compiling $< to $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/obj/%.S.o: %.S
	@echo "Assembling $< to $@"
	@mkdir -p "$(dir $@)"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/lib/obj/%.asm.o: %.asm
	@echo "Assembling $< to $@"
	@mkdir -p "$(dir $@)"
	@nasm $(NASMFLAGS) $< -o $@

clean:
	@rm -rf $(LIBK) $(OBJ)