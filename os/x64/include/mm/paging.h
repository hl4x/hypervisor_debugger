#ifndef __PAGING_H
#define __PAGING_H

#include <stdint.h>

#define FOUR_K (1ULL << 12)
#define TWO_MB (1ULL << 21)

extern uint64_t pml4_base[512];
extern uint64_t pdp_base[512];
// pml4[0]->pdp[0]->pd[0...511] is populated in boot.s
extern uint64_t pd_base[512];

#endif // __PAGING_H
