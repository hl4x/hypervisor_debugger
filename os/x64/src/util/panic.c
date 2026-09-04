#include <stdarg.h>

#include "util/panic.h"
#include "util/printf.h"

__attribute__((noreturn)) void panic(const char *fmt, ...)
{
    // disable interrupts
    __asm__ volatile ("cli");

    printf("\n\n");
    printf("=================================================================\n");
    printf("                        KERNEL PANIC                             \n");
    printf("=================================================================\n");

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args); 

    for (;;) {
        __asm__ volatile ("hlt");
    }
}
