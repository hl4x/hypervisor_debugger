#ifndef __MM_H
#define __MM_H

extern uint8_t _kernel_end;

void init_bump_allocator();
void* bump_alloc(uint64_t size);

#endif // __MM_H