#ifndef __MSR_H
#define __MSR_H

#include <stdint.h>

#define MSR_VM_CR 0xc0010114

#define MSR_VM_CR_SVMDIS (1U << 4)

static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t lo, hi;
    __asm__ volatile ("rdmsr"
                      : "=a"(lo), "=d"(hi)
                      : "c"(msr));
    return ((uint64_t)hi << 32) | lo;
}

#endif // __MSR_H
