#include <stdint.h>
#include "kernel.h"

void kernel_main(void)
{
    for (;;) {
        __asm__ volatile ("hlt");
    }
}