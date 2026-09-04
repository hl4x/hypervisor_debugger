#include <stdint.h>

#include "mm/mm.h"
#include "kernel/multiboot.h"
#include "util/bug.h"

uint64_t bump_ptr;
uint64_t range_end;

void parse_multiboot_memory(uint32_t mb_addr)
{
    multiboot_info_t *multiboot_info = (multiboot_info_t*)(uint64_t)mb_addr;
    if (!(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP)) {
        return;
    }

    uint32_t mem_length = multiboot_info->mmap_length;
    uint64_t offset = 0;
    while (offset < mem_length) {
        uint32_t current_addr = multiboot_info->mmap_addr + offset;
        multiboot_memory_map_t *mmap_entry = (multiboot_memory_map_t*)(uint64_t)current_addr;
        if (mmap_entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t mmap_entry_start = mmap_entry->addr;
            uint64_t mmap_entry_end = mmap_entry_start + mmap_entry->len;
            uint64_t kernel_end_addr = (uint64_t)&_kernel_end;
            if (kernel_end_addr >= mmap_entry_start &&
                kernel_end_addr < mmap_entry_end) {
                    range_end = mmap_entry_end;
                    bump_ptr = (uint64_t)&_kernel_end;
                    break;
                }
        }
        offset += mmap_entry->size + sizeof(mmap_entry->size);
    }
}

void* bump_alloc(uint64_t size)
{
    BUG_ON(!bump_ptr || !range_end, "bump_ptr and range_end not initialized");
    // Align size to the next page boundary (e.g. size = 10 -> 4096)
    size = (size + 0xFFF) & ~0xFFF;
    BUG_ON(bump_ptr + size > range_end, "Out of physical memory in bump allocator, Bump Pointer %lxh", bump_ptr);
    uint64_t ptr = bump_ptr;
    bump_ptr += size;
    return (void*)ptr;
}