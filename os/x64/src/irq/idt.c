#include <stdint.h>

#include "util/printf.h"
#include "irq/idt.h"
#include "irq/idt_vectors.h"

// Place the idt in .bss with 16-byte alignment
gate_desc idt[256] __attribute__((aligned(16)));
struct desc_ptr idtr;

void isr_handler(struct isr_save_state *frame)
{
    printf("VEC: %lu, ERR: 0x%lx, RIP: 0x%lx\n", 
           frame->intr_vector_nr, 
           frame->err_code, 
           frame->iret.old_rip);

    if (frame->intr_vector_nr == 3) {
        frame->iret.old_rip += 1;
    }
}

void init_idt(void)
{
    for (uint64_t i = 0; i < 256; i++) {
        unsigned type   = IS_TRAP_GATE(i) ? TRAP_GATE : INTR_GATE;
        unsigned dpl    = 0; 
        unsigned ist    = IS_IST_ONE(i) ? 1 : 0;
        pack_gate(&idt[i], type, (uint64_t)isr_stub_table[i], dpl, ist); 
    }

    idtr.size    = sizeof(idt) - 1;
    idtr.address = (uint64_t)&idt;

    load_idt(&idtr);
}