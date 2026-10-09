#ifndef __MM_H
#define __MM_H

#include <stdint.h>
#include <stddef.h>
#include <ptrcheck.h>

#include <kernel/multiboot.h>
#include <util/bug.h>

#define USABLE_RANGE_MAX 64

extern uint8_t _kernel_end;

struct range
{
    uint64_t start, end;
} __attribute((packed));
typedef struct range range_t;

static inline
void *__sized_by(size) phys_map(uint64_t pa, uint64_t size)
{
    BUG_ON(pa == 0, "phys_map: null physicial address");
    BUG_ON(pa + size < pa, "phys_map: overflow, pa=0x%lx size=0x%lx", pa, size);
    return __unsafe_forge_bidi_indexable(void *, pa, size);
}

void parse_multiboot_memory(multiboot_info_t *mb);
void setup_runtime_pages(void);
void *__sized_by(size) bump_alloc(uint64_t size);
void *__sized_by(size) bump_alloc_aligned(uint64_t size, uint64_t align);
void *__sized_by(n) memset(void *__sized_by(n) s, uint8_t c, size_t n);
void *__sized_by(n) memcpy(void *__sized_by(n) d, const void *__sized_by(n) s, size_t n);

#endif // __MM_H
