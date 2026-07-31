OUTPUT := $(abspath .output)
TOOLCHAIN := $(abspath ../../toolchain)
CC := $(abspath $(TOOLCHAIN)/i686/bin/i686-elf-gcc)
Q := @ 
msg := @printf '  %-8s %s%s\n'					\
	      "$(1)"						\
	      "$(patsubst $(abspath $(OUTPUT))/%,%,$(2))"	\
	      "$(if $(3), $(3))";
MAKEFLAGS += --no-print-directory
CPPFLAGS := -I include
CFLAGS := -g -ffreestanding 
LDFLAGS := -nostdlib -T linker.ld
LDLIBS := -lgcc
OBJS := \
	$(OUTPUT)/boot.o \
	$(OUTPUT)/main.o \
	$(OUTPUT)/printf.o 

.PHONY: all
all: x32_kernel

x32_kernel: $(OBJS)
	$(call msg,LD,$@)
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) -std=gnu99 -O2 -Wall -Wextra -fno-pie -o $@ $^ $(LDLIBS)

$(OUTPUT)/%.o: src/%.c | $(OUTPUT)
	$(call msg,CC,$@)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OUTPUT)/boot.o: src/boot.s | $(OUTPUT)
	$(call msg,ASM,$@)
	$(Q)nasm -felf32 -o $@ $<

$(OUTPUT):
	$(Q)mkdir -p $@

.PHONY: clean
clean:
	$(call msg,CLEAN)
	$(Q)rm -rf $(OUTPUT)