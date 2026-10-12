#ifndef __KERNEL_H
#define __KERNEL_H

#include <stdint.h>

#include "kernel/multiboot.h"

#define CLEAR_SCREEN "\e[1;1H\e[2J"

extern void *stack64_top;
extern void *stack64_bottom;

void init64(multiboot_info_t *mb);
void kernel_main(multiboot_info_t *mb);

#endif // __KERNEL_H
