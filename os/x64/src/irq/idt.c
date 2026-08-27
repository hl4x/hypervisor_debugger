#include <stdint.h>

#include "util/printf.h"
#include "irq/idt.h"

// Place the idt in .bss with 16-byte alignment
gate_desc idt[256] __attribute__((aligned(16)));
struct desc_ptr idtr;

void isr_handler(struct isr_save_state *frame)
{
    uint64_t vec = frame->intr_vector_nr;
    uint64_t err = frame->err_code;
    uint64_t rip = frame->iret.old_rip;

    printf("VEC: %d, ERR: %d, RIP: %d", vec, err, rip);
}

void init_idt(void)
{
    for (uint64_t i = 0; i < 256; i++) {
        unsigned type   = (TRAP_TYPE_MASK >> i) & 1 ? TRAP_GATE : INTR_GATE;
        unsigned dpl    = 0; 
        unsigned ist    = (IST_ONE_MASK >> i) & 1 ? 1 : 0;
        pack_gate(&idt[i], type, (uint64_t)isr_stub_table[i], dpl, ist); 
    }

    idtr.size    = sizeof(idt) - 1;
    idtr.address = (uint64_t) &idt;

    load_idt(&idtr);
}