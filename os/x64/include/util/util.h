#ifndef __UTIL_H
#define __UTIL_H

#include <stdint.h>
#include <stdbool.h>

#include "util/bug.h"
#include "io/port_io.h"

#define PS2_TIMEOUT 100000U

static inline
uint64_t max(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}

static inline
uint64_t align_up(uint64_t v, uint64_t a)
{
    uint64_t next_boundary = v+a-1;
    BUG_ON(next_boundary < v,
           "v overflow on next boundary, v=0x%lx next_boundary=0x%lx",
           v, next_boundary);
    BUG_ON(a-1 > a, "align-to size underflow, a=0x%lx",
           a);
    BUG_ON(a & (a-1), "align-to is not power of 2, a=0x%lx",
           a);
    return next_boundary & ~(a-1);
}

static inline
uint64_t align_down(uint64_t v, uint64_t a)
{
    BUG_ON(a-1 > a, "align to size underflow, a=0x%lx",
           a);
    BUG_ON(a & (a-1), "align to is not power of 2, a=0x%lx",
           a);
    return v & ~(a-1);
}

static inline
uint64_t align_up_shift(uint64_t v, uint64_t n)
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

static inline
uint64_t align_down_shift(uint64_t v, uint64_t n)
{
    BUG_ON(n >= 64,
           "can't align to greater than 64 bits, n=%lu",
           n);
    return v & ~((1ULL<<n)-1);
}

static inline
bool ps2_wait_ibf_clear(void)
{
    for (uint32_t i = 0; i < PS2_TIMEOUT; i++)
        if (!(inb(0x64) & 0x02)) // input buffer full
            return true;
    return false;
}

static inline
bool ps2_wait_obf_set(void)
{
    for (uint32_t i = 0; i < PS2_TIMEOUT; i++)
        if (inb(0x64) & 0x01) // output buffer full
            return true;
    return false;
}

static inline
bool flash_keyboard_leds(void)
{
    if (!ps2_wait_ibf_clear())
        return false;
    outb(0x60, 0xED);
    if (!ps2_wait_obf_set())
        return false;
    if (inb(0x60) != 0xFA) // ACK
        return false;
    if (!ps2_wait_ibf_clear())
        return false;
    outb(0x60, 0x07); // all LEDs on
    if (!ps2_wait_obf_set())
        return false;
    (void)inb(0x60); // ACK
    return true;
}

#endif // __UTIL_H
