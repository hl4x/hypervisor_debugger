#include <stdint.h>
#include <stddef.h>
#include <ptrcheck.h>

#include "mm/mm.h"
#include "mm/paging.h"
#include "kernel/multiboot.h"
#include "util/bug.h"
#include "util/util.h"

// static so these vars are only visible internally for mm.c
static uint64_t bump_ptr;
static uint64_t range_end;

// 64 entries should be enough
static range_t usable_ranges[USABLE_RANGE_MAX];
static uint64_t usable_range_count;
static uint64_t range_idx_parse, range_idx_alloc;

static uint64_t kernel_end_addr;

static uint64_t pml4_addr;

void parse_multiboot_memory(multiboot_info_t *mb)
{
    BUG_ON(!(mb->flags & MULTIBOOT_INFO_MEM_MAP),
            "parse_multiboot_memory: memory map not full, mb->flags=0x%x",
            mb->flags);

    range_idx_parse = 0;
    kernel_end_addr = (uint64_t)&_kernel_end;
    usable_range_count = 0;

    uint32_t mem_length = mb->mmap_length;
    uint64_t offset = 0;
    uint8_t *mmap = phys_map(mb->mmap_addr, mb->mmap_length);
    while (offset < mem_length) {
        BUG_ON(mem_length - offset < sizeof(multiboot_memory_map_t),
               "parse_multiboot_memory: trailing partial mmap entry, offset 0x%lx len 0x%x",
               offset, mem_length);

        multiboot_memory_map_t *mmap_entry = (multiboot_memory_map_t *)(mmap + offset);
        if (mmap_entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            uint64_t mmap_entry_start = mmap_entry->addr;
            uint64_t mmap_entry_end = mmap_entry_start + mmap_entry->len;

            BUG_ON(mmap_entry_end < mmap_entry_start,
                   "parse_multiboot_memory: mmap_entry_end overflow, mmap_entry_start 0x%lx mmap_entry_end 0x%lx",
                   mmap_entry_start, mmap_entry_end);

            uint64_t s = align_up(max(mmap_entry_start, kernel_end_addr), FOUR_K);
            uint64_t e = align_down(mmap_entry_end, FOUR_K);

            // kernel_end_addr can be greater than the particular mmap_entry_end
            if (s >= e)
                goto inc_offset;

            BUG_ON(range_idx_parse >= USABLE_RANGE_MAX,
                   "too many memory mappings found in multiboot info, consider increasing USABLE_RANGE_MAX range_idx_parse=%lu, USABLE_RANGE_MAX=%d",
                   range_idx_parse, USABLE_RANGE_MAX);

            usable_ranges[range_idx_parse].start = s;
            usable_ranges[range_idx_parse].end = e;
            range_idx_parse++;
            usable_range_count++;
        }

inc_offset:
        // https://elixir.bootlin.com/grub/grub-2.14/source/grub-core/loader/i386/multiboot_mbi.c#L274
        uint64_t next_offset = offset + mmap_entry->size + sizeof(mmap_entry->size);
        BUG_ON(next_offset < offset,
               "parse_multiboot_memory: offset overflow, offset 0x%lx size 0x%x",
               offset, mmap_entry->size);
        BUG_ON(next_offset > mem_length,
               "parse_multiboot_memory: entry overruns map, offset 0x%lx size 0x%x len 0x%x",
               offset, mmap_entry->size, mem_length);
        offset = next_offset;
    }

    BUG_ON(usable_range_count == 0,
           "no usable ranges found, usable_range_count=%lu",
           usable_range_count);
    range_idx_alloc = 0;
    bump_ptr = usable_ranges[range_idx_alloc].start;
    range_end = usable_ranges[range_idx_alloc].end;
}

void setup_runtime_pages(void)
{
    uint64_t *__counted_by(512) pml4 = bump_alloc_aligned(FOUR_K, FOUR_K);
    memset(pml4, 0, FOUR_K);
    pml4_addr = (uint64_t)pml4;

    // add one for the kernel itself
    uint64_t map_list_count = usable_range_count + 1;
    uint64_t map_list_size = sizeof(*usable_ranges) * map_list_count;
    range_t *map_list = bump_alloc_aligned(map_list_size, sizeof(range_t));

    // kernel entry + usable_ranges
    map_list[0].start = ONE_MB;
    map_list[0].end = align_up(kernel_end_addr, FOUR_K);
    memcpy(&map_list[1], usable_ranges, sizeof(*usable_ranges) * usable_range_count);

    for (uint64_t i = 0; i < map_list_count; i++) {
        uint64_t entry_idx = 0;

        uint64_t cur_addr = map_list[i].start;
        uint64_t end = map_list[i].end;

        while (cur_addr < end) {
            uint64_t *__counted_by(512) table = pml4;

            // 4-level paging with 4Kbyte pages
            // walk down from pml4->pdp->pd
            for (uint32_t level = 3; level != 0; level--) {
                entry_idx = level_index(cur_addr, level);
                uint64_t *__sized_by(8) entry = &table[entry_idx];

                if (!(*entry & PRESENT)) {
                    uint64_t *__counted_by(512) fresh = bump_alloc_aligned(FOUR_K, FOUR_K);
                    memset(fresh, 0, FOUR_K);
                    table[entry_idx] = (uint64_t) fresh | PRESENT | READ_WRITE;
                    // descend into the lower table
                    table = fresh;
                }
                else {
                    table = phys_map(*entry & PTE_ADDR_MASK, FOUR_K);
                }
            }
            // pte
            entry_idx = level_index(cur_addr, 0);
            table[entry_idx] = (uint64_t) cur_addr | PRESENT | READ_WRITE;

            cur_addr += FOUR_K;
        }
    }

    // load cr3 with new pml4 base
    asm __volatile__ ("mov %0, %%cr3" : : "r"((uint64_t)pml4) : "memory");
}

static void try_get_next_usable_range(void)
{
    range_idx_alloc++;
    BUG_ON(range_idx_alloc >= usable_range_count,
           "Out of usable memory ranges, range_idx_alloc=%lu usable_range_count=%lu",
           range_idx_alloc, usable_range_count);
    bump_ptr = usable_ranges[range_idx_alloc].start;
    range_end = usable_ranges[range_idx_alloc].end;
}

void *__sized_by(size) bump_alloc_aligned(uint64_t size, uint64_t align)
{
    BUG_ON(!bump_ptr || !range_end, "bump_ptr and range_end not initialized");

    // aligned size to next 4096 boundary
    uint64_t size_aligned = align_up(size, FOUR_K);

retry:
    // make sure bump_ptr is aligned to the request b/c ptr is copied from bump_ptr
    bump_ptr = align_up(bump_ptr, align);
    BUG_ON(bump_ptr + size_aligned < bump_ptr, "bump_alloc: bump_ptr overflow, bump_ptr = 0x%lx", bump_ptr);
    // if out of phys memory in this range, look for another available range
    if (bump_ptr + size_aligned > range_end) {
        try_get_next_usable_range();
        // re-align our new bump_ptr
        goto retry;
    }
    uint64_t ptr = bump_ptr;
    bump_ptr += size_aligned;

    return phys_map(ptr, size);
}

void *__sized_by(size) bump_alloc(uint64_t size)
{
    // default page granularity is 4K
    // use bump_alloc_aligned(size, TWO_MB) explicitly for PS=1 entries
    // 4K granularity shouldn't break future 2MB allocs as 2MB % 4K == 0
    return bump_alloc_aligned(size, FOUR_K);
}

void *__sized_by(n) memset(void *__sized_by(n) s, uint8_t c, size_t n)
{
    uint8_t *local_s = s;
    size_t i = 0;
    while (i < n)
        local_s[i++] = c;
    return s;
}

void *__sized_by(n) memcpy(void *__sized_by(n) d, const void *__sized_by(n) s, size_t n)
{
    uint8_t *local_d = d;
    const uint8_t *local_s = s;
    for (size_t i = 0; i < n; i++)
        local_d[i] = local_s[i];
    return d;
}
