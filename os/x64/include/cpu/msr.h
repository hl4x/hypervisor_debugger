#ifndef __MSR_H
#define __MSR_H

#include <stdint.h>

#define MSR_VM_CR 0xc0010114

#define MSR_VM_CR_SVMDIS (1U << 4)

static inline
uint64_t rdmsr(uint32_t msr)
{
    uint32_t lo, hi;
    __asm__ volatile ("rdmsr"
                      : "=a"(lo), "=d"(hi) /* output */
                      : "c"(msr) /* input */
                      : "memory");
    return ((uint64_t)hi << 32) | lo;
}

static inline
void wrmsr(uint32_t msr, uint64_t val)
{
    uint32_t lo = (uint32_t)val;
    uint32_t hi = (uint32_t)(val >> 32);
    __asm__ volatile ("wrmsr"
                      :
                      : "c"(msr), "d"(hi), "a"(lo)
                      : "memory");
}

#endif // __MSR_H
