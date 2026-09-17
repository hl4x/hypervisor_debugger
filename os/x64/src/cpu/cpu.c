#include <stdint.h>

#include "cpu/cpu.h"

void cpuid(uint32_t function_number, uint32_t optional_input,
           uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
    __asm__ volatile ("cpuid"
                      : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
                      : "0" (function_number), "2" (optional_input)
    );
}
