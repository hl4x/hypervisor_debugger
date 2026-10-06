#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>

#include "util/bug.h"

static inline uint64_t max(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}

static inline uint64_t align_up(uint64_t v, uint64_t a)
{
    uint64_t next_boundary = v+a-1;
    BUG_ON(next_boundary < v,
           "v overflow on next boundary, v=0x%lx next_boundary=0x%lx",
           v, next_boundary);
    BUG_ON(a-1 > a, "align to size underflow, a=0x%lx",
           a);
    BUG_ON(a & (a-1), "align to is not power of 2, a=0x%lx",
           a);
    return next_boundary & ~(a-1);
}

static inline uint64_t align_down(uint64_t v, uint64_t a)
{
    BUG_ON(a-1 > a, "align to size underflow, a=0x%lx",
           a);
    BUG_ON(a & (a-1), "align to is not power of 2, a=0x%lx",
           a);
    return v & ~(a-1);
}

static inline uint64_t align_up_shift(uint64_t v, uint64_t n)
{
    BUG_ON(n >= 64,
           "can't align to greater than 64 bits, n=%lu",
           n);
    uint64_t n_ones = (1ULL << n) - 1;
    uint64_t next_boundary = v + n_ones;
    BUG_ON(next_boundary < v,
           "v overflow on next boundary, v=0x%lx next_boundary=0x%lx",
           v, next_boundary);
    return next_boundary & ~(n_ones);
}

static inline uint64_t align_down_shift(uint64_t v, uint64_t n)
{
    BUG_ON(n >= 64,
           "can't align to greater than 64 bits, n=%lu",
           n);
    return v & ~((1ULL<<n)-1);
}

#endif // __UTIL_H
