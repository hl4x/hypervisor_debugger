OUTPUT := $(abspath .output)
KERNEL := $(OUTPUT)/kernel
ISO_DIR := $(OUTPUT)/isodir
ISO := $(OUTPUT)/hypervisor_debugger.iso

TOOLCHAIN := $(abspath ../toolchain)
CC := $(abspath $(TOOLCHAIN)/x86_64/bin/x86_64-elf-gcc)

MAKEFLAGS += --no-print-directory
CPPFLAGS := -Ix64/include
CFLAGS := -g -ffreestanding -std=gnu99 -O2 -Wall -Wextra -fno-pie
LDFLAGS := -nostdlib -T linker.ld
LDLIBS := -lgcc

SRCS := $(shell find x64/src -name '*.c')
OBJS := $(patsubst x64/src/%.c,$(OUTPUT)/x64/%.o,$(SRCS))
OBJS := \
	$(OUTPUT)/x64/src/boot/boot.o \
	$(OBJS)

Q := @
msg := @printf '  %-8s %s%s\n'					\
	      "$(1)"						\
	      "$(patsubst $(abspath $(OUTPUT))/%,%,$(2))"	\
	      "$(if $(3), $(3))";

.PHONY: all
all: $(ISO) 

$(KERNEL): $(OBJS)
	$(call msg,LD,$@)
	$(Q)$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(ISO): $(KERNEL)
	$(call msg,ISO,$@)
	$(Q)mkdir -p $(ISO_DIR)/boot/grub
	$(Q)cp $(KERNEL) $(ISO_DIR)/boot/kernel
	$(Q)echo 'set timeout=0; set default=0; menuentry "hypervisor_debugger" { multiboot /boot/kernel }' > $(ISO_DIR)/boot/grub/grub.cfg
	$(Q)grub-mkrescue -o $(ISO) $(ISO_DIR)

$(OUTPUT)/x64/src/boot/boot.o: x64/src/boot/boot.s 
	@mkdir -p $(dir $@)
	$(call msg,ASM,$@)
	$(Q)nasm -felf64 -o $@ $<

$(OUTPUT)/x64/%.o: x64/src/%.c
	@mkdir -p $(dir $@)
	$(call msg,CC64, $@)
	$(Q)$(CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

DEPS := $(OBJS:.o=.d)
-include $(DEPS)

.PHONY: clean
clean:
	$(call msg,CLEAN)
	$(Q)rm -rf $(OUTPUT)

.PHONY: run
run: $(ISO)
	$(call msg,QEMU,Starting VM...)
	$(Q)qemu-system-x86_64 \
		-smp 2 \
		-cpu host \
		-m 4G \
		-cdrom $(ISO) \
		-enable-kvm \
		-nographic \
		-monitor telnet:127.0.0.1:1235,server,nowait

.PHONY: debug 
debug: $(ISO)
	$(call msg,QEMU,Starting VM...)
	$(Q)qemu-system-x86_64 \
		-smp 2 \
		-cpu host \
		-m 4G \
		-cdrom $(ISO) \
		-enable-kvm \
		-nographic \
		-monitor telnet:127.0.0.1:1235,server,nowait \
		-s -S