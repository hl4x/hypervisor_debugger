#ifndef __MM_H
#define __MM_H

extern uint8_t _kernel_end;

void parse_multiboot_memory(uint32_t mb_addr);
void* bump_alloc(uint64_t size);

#endif // __MM_H