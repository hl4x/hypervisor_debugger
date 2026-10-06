#ifndef __MM_H
#define __MM_H

#include <stdint.h>
#include <stddef.h>
#include <ptrcheck.h>

#define USABLE_RANGE_MAX 64

struct range
{
    uint64_t start, end;
} __attribute((packed));
typedef struct range range_t;

extern uint8_t _kernel_end;

void parse_multiboot_memory(uint32_t mb_addr);
void setup_runtime_pages();
void *__sized_by(size) bump_alloc(uint64_t size);
void *__sized_by(size) bump_alloc_aligned(uint64_t size, uint64_t align);
void *__sized_by(n) memset(void *__sized_by(n) s, uint8_t c, size_t n);

#endif // __MM_H
