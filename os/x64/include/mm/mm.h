#ifndef __MM_H
#define __MM_H

#include <stdint.h>
#include <stddef.h>
#include <ptrcheck.h>

#include <kernel/multiboot.h>
#include <util/bug.h>

#define USABLE_RANGE_MAX 64
#define MODULES_MAX 64

extern uint64_t pml4_addr;
extern uint8_t _kernel_end;

struct range
{
    uint64_t start, end;
} __attribute((packed));
typedef struct range range_t;

static inline
void *__sized_by(size) phys_map_bytes(uint64_t pa, uint64_t size)
{
    BUG_ON(pa == 0, "phys_map: null physicial address");
    BUG_ON(pa + size < pa, "phys_map: overflow, pa=0x%lx size=0x%lx", pa, size);
    return __unsafe_forge_bidi_indexable(void *, pa, size);
}

#define phys_map(type, pa, count)               \
    ((type *__counted_by(count))phys_map_bytes((pa),    \
        sizeof(type) * (uint64_t)(count)))

static inline
void reload_cr3(void)
{
    BUG_ON(!pml4_addr,
           "reload_cr3: pml4_addr not set");
    asm __volatile__ ("mov %0, %%cr3" : : "r"(pml4_addr) : "memory");
}

void parse_multiboot_info(multiboot_info_t *mb);
multiboot_module_t *boot_module_get(uint32_t idx);
void map_addr(uint64_t addr);
void map_range(uint64_t start, uint64_t end);
void map_available_ranges(void);
void *__sized_by(size) bump_alloc(uint64_t size);
void *__sized_by(size) bump_alloc_aligned(uint64_t size, uint64_t align);
void *__sized_by(n) memset(void *__sized_by(n) s, uint8_t c, size_t n);
void *__sized_by(n) memcpy(void *__sized_by(n) d, const void *__sized_by(n) s, size_t n);

#endif // __MM_H
