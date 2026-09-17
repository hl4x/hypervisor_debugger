#ifndef __CPU_H
#define __CPU_H

#include <stdint.h>

extern uint32_t cpuid_supported(void);

void cpuid(uint32_t function_number, uint32_t optional_input,
           uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d);

#endif // __CPU_H
