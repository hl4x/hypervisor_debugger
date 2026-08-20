OUTPUT := $(abspath .output)

TOOLCHAIN := $(abspath ../toolchain)
CC32 := $(abspath $(TOOLCHAIN)/i686/bin/i686-elf-gcc)
CC64 := $(abspath $(TOOLCHAIN)/x86_64/bin/x86_64-elf-gcc)

Q := @
msg := @printf '  %-8s %s%s\n'					\
	      "$(1)"						\
	      "$(patsubst $(abspath $(OUTPUT))/%,%,$(2))"	\
	      "$(if $(3), $(3))";

MAKEFLAGS += --no-print-directory
CPPFLAGS32 := -Ix32/include
CPPFLAGS64 := -Ix64/include
CFLAGS := -g -ffreestanding -std=gnu99 -O2 -Wall -Wextra -fno-pie
LDFLAGS := -nostdlib -T linker.ld
LDLIBS := -lgcc

X32_SRCS := $(wildcard x32/src/*.c)
X32_OBJS := $(patsubst x32/src/%.c,$(OUTPUT)/x32/%.o,$(X32_SRCS))

X64_SRCS := $(wildcard x64/src/*.c)
X64_OBJS := $(patsubst x64/src/%.c,$(OUTPUT)/x64/%.o,$(X64_SRCS))

OBJS := \
	$(OUTPUT)/x32/boot.o \
	$(X32_OBJS) \

.PHONY: all
all: kernel

kernel: $(OBJS)
	$(call msg,LD,$@)
	$(Q)$(CC32) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(OUTPUT)/x32/boot.o: x32/src/boot.s | $(OUTPUT)/x32
	$(call msg,ASM,$@)
	$(Q)nasm -felf32 -o $@ $<

$(OUTPUT)/x32/%.o: x32/src/%.c | $(OUTPUT)/x32
	$(call msg,CC32,$@)
	$(Q)$(CC32) $(CPPFLAGS32) $(CFLAGS) -c $< -o $@

$(OUTPUT)/x64/%.o: x64/src/%.c | $(OUTPUT)/x64
	$(call msg,CC64, $@)
	$(Q)$(CC64) $(CPPFLAGS64) $(CFLAGS) -c $< -o $@

$(OUTPUT)/x32 $(OUTPUT)/x64:
	$(Q)mkdir -p $@

.PHONY: clean
clean:
	$(call msg,CLEAN)
	$(Q)rm -rf $(OUTPUT)
