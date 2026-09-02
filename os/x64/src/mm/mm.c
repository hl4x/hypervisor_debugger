#include <stdint.h>

#include "mm/mm.h"

uint64_t bump_ptr;

void init_bump_allocator()
{
    bump_ptr = (uint64_t)&_kernel_end;
}

void* bump_alloc(uint64_t size)
{
    // Align size to the next page boundary (e.g. size = 10 -> 4096)
    size = (size + 0xFFF) & ~0xFFF;
    
    // TODO: get range_end for below from multiboot info
    //if (bump_ptr + size > range_end) {
    //    panic("Out of physical memory in bump allocator")
    //}
    
    uint64_t ptr = bump_ptr;
    bump_ptr += size;
    return (void*)ptr;
}