#include <stdint.h>
#include <ptrcheck.h>

#include "mm/mm.h"
#include "kernel/multiboot.h"
#include "util/bug.h"

uint64_t bump_ptr;
uint64_t range_end;

void parse_multiboot_memory(uint32_t mb_addr)
{
    multiboot_info_t *multiboot_info = __unsafe_forge_single(multiboot_info_t*, (uint64_t)mb_addr);

    if (!(multiboot_info->flags & MULTIBOOT_INFO_MEM_MAP)) {
        return;
    }

    uint32_t mem_length = multiboot_info->mmap_length;
    uint64_t offset = 0;
    while (offset < mem_length) {
        BUG_ON(mem_length - offset < sizeof(multiboot_memory_map_t),
               "parse_multiboot_memory: trailing partial mmap entry, offset 0x%lx len 0x%x",
               offset, mem_length);
        uint64_t current_addr = (uint64_t)multiboot_info->mmap_addr + offset;
        multiboot_memory_map_t *mmap_entry = __unsafe_forge_single(multiboot_memory_map_t*, current_addr);
        if (mmap_entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t mmap_entry_start = mmap_entry->addr;
            uint64_t mmap_entry_end = mmap_entry_start + mmap_entry->len;
            BUG_ON(mmap_entry_end < mmap_entry_start,
                   "parse_multiboot_memory: mmap_entry_end overflow, mmap_entry_start 0x%lx mmap_entry_end 0x%lx",
                   mmap_entry_start, mmap_entry_end);
            uint64_t kernel_end_addr = (uint64_t)&_kernel_end;
            if (kernel_end_addr >= mmap_entry_start &&
                kernel_end_addr < mmap_entry_end) {
                    range_end = mmap_entry_end;
                    bump_ptr = (uint64_t)&_kernel_end;
                    break;
                }
        }
        uint64_t next_offset = offset + mmap_entry->size + sizeof(mmap_entry->size);
        BUG_ON(next_offset < offset,
               "parse_multiboot_memory: offset overflow, offset 0x%lx size 0x%x",
               offset, mmap_entry->size);
        BUG_ON(next_offset > mem_length,
               "parse_multiboot_memory: entry overruns map, offset 0x%lx size 0x%x len 0x%x",
               offset, mmap_entry->size, mem_length);
        offset = next_offset;
    }
}

void *__sized_by(size) bump_alloc(uint64_t size)
{
    BUG_ON(!bump_ptr || !range_end, "bump_ptr and range_end not initialized");
    // Align size to the next page boundary (e.g. size = 10 -> size_aligned = 4096)
    uint64_t size_aligned = (size + 0xFFF) & ~0xFFF;
    BUG_ON(size_aligned < size, "bump_alloc: aligned size overflow, size=0x%lx size_aligned=0x%lx", size, size_aligned);
    BUG_ON(bump_ptr + size_aligned < bump_ptr, "bump_alloc: bump_ptr overflow, bump_ptr = 0x%lx", bump_ptr);
    BUG_ON(bump_ptr + size_aligned > range_end, "Out of physical memory in bump allocator, Bump Pointer %lxh", bump_ptr);
    uint64_t ptr = bump_ptr;
    bump_ptr += size_aligned;
    return __unsafe_forge_bidi_indexable(void*, ptr, size);
}
