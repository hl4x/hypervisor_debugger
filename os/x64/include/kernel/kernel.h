#ifndef __KERNEL_H
#define __KERNEL_H

#include "kernel/multiboot.h"

#define CLEAR_SCREEN "\e[1;1H\e[2J"

void init64();
void kernel_main(uint32_t mb_addr);

#endif // __KERNEL_H