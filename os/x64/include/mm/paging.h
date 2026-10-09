#ifndef __PAGING_H
#define __PAGING_H

#include <stdint.h>

#define FOUR_K (1ULL << 12)
#define ONE_MB (1ULL << 20)
#define TWO_MB (1ULL << 21)
#define ONE_G (1ULL << 30)

// page permissions
#define PRESENT (1ULL << 0)
#define READ_WRITE (1ULL << 1)
#define PDE_PS (1ULL << 7)

// Figure 5-17. 4-Kbyte Page Translation—Long Mode 4-Level Paging (p.142)
#define PAGE_SHIFT 12
#define IDX_BITS 9
#define ENTRIES (1ULL << IDX_BITS)
#define IDX_MASK (ENTRIES - 1)
#define LEVELS 4

// bits 51:12 of 4 Kbyte page-table entries
#define PTE_ADDR_MASK 0x000FFFFFFFFFF000ULL

static inline
uint32_t level_shift (uint32_t level)
{
    return PAGE_SHIFT + level * IDX_BITS;
}

static inline
uint64_t level_index (uint64_t addr, uint32_t level)
{
    return (addr >> level_shift(level)) & IDX_MASK;
}

#endif // __PAGING_H
