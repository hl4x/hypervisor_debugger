#ifndef __PAGING_H
#define __PAGING_H

#include <stdint.h>

void* init_page_tables(void);

#define PG_PRESENT  (1 << 0)    // 0b01
#define PG_WRITE    (1 << 1)    // 0b10
#define PG_TABLE_ENTRY (ptr) (((uint64_t_)ptr) | PG_PRESENT | PG_WRITE )

#define PAGE_SIZE 4096

#endif // __PAGING_H