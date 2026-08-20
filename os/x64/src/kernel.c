#include <stdint.h>
#include "kernel.h"

__attribute__((section(".code64")))
void kernel_main(void)
{
    for (;;) {
        __asm__ volatile ("hlt");
    }
}