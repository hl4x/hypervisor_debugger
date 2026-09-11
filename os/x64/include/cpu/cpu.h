#ifndef __MSR_H
#define __MSR_H

#include <stdint.h>

extern uint32_t cpuid_supported(void);

void cpuid(uint32_t function, uint32_t subfunction,
           uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d)
{
    __asm__ volatile ("cpuid"
                      : "=a" (*a), "=b" (*b), "=c" (*c), "=d" (*d)
                      : "0" (function), "2" (subfunction)
    );
}

#endif // __MSR_H
