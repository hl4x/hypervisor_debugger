#include <stdint.h>
#include "paging.h"

extern uint64_t pml4_base;
extern uint64_t pdpe_base;
extern uint64_t pde_base;
extern uint64_t pte_base;

void* init_page_tables(void)
{
}