#include <stdint.h>

#include "util/printf.h"
#include "irq/idt.h"

// Place the idt in .bss with 16-byte alignment
struct idt_entry64 idt[256] __attribute__((aligned(16)));

void isr_handler(struct isr_save_state *frame)
{
    uint64_t vec = frame->intr_vector_nr;
    uint64_t err = frame->err_code;
    uint64_t rip = frame->iret.old_rip;

    printf("VEC: %d, ERR: %d, RIP: %d", vec, err, rip);
}

void initialize_idt(void)
{
}