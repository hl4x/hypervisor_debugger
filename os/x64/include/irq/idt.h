#ifndef __IDT_H
#define __IDT_H

#include <stdint.h>

#include "irq/idt_vectors.h"

// Table 4-6. System-Segment Descriptor Types—Long Mode (continued)
#define TRAP_GATE   0x0F
#define INTR_GATE   0x0E

//#define TRAP_TYPE_MASK  (1ULL << IDT_DOUBLE_FAULT | 1ULL << IDT_OVERFLOW )
//#define IST_ONE_MASK    (1ULL << IDT_DOUBLE_FAULT | 1ULL << IDT_NMI_INTERRUPT | 1ULL << IDT_VMM_COMMUNICATION )
#define IS_TRAP_GATE(v) ((v) == IDT_BREAKPOINT || (v) == IDT_OVERFLOW)
#define IS_IST_ONE(v)   ((v) == IDT_DOUBLE_FAULT || \
                         (v) == IDT_NMI_INTERRUPT || \
                         (v) == IDT_VMM_COMMUNICATION)

#define __KERNEL_CS 0x8

struct idt_bits {
    uint16_t    ist         : 3,
                reserved    : 5,
                type        : 4,
                zero        : 1,
                dpl         : 2,
                p           : 1;
} __attribute__((packed));

// See figure 4-24 from AMD64 Volume 2 Manual
struct gate_struct {
    uint16_t offset_low; 
    uint16_t segment;
    struct idt_bits bits;
    uint16_t offset_middle;
    uint32_t offset_high;
    uint32_t reserved_zero;
} __attribute__((packed));

typedef struct gate_struct gate_desc;

// See figure 8-14
struct iret_stack {
    uint64_t old_rip;
    uint64_t old_cs;
    uint64_t old_rflags;
    uint64_t old_rsp;
    uint64_t old_ss;
} __attribute__((packed));

struct  isr_save_state {
    // These need to be defined to match PUSH_ALL and POP_ALL stack layout
    // lower memm; r15 is low (rsp+0)
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8; 
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax; 

    uint64_t intr_vector_nr;
    uint64_t err_code;

    struct iret_stack iret;
    // higher mem
} __attribute__((packed));

struct desc_ptr {
    uint16_t size;
    uint64_t address;
} __attribute__((packed));

static inline void pack_gate(gate_desc *gate, unsigned type, uint64_t func, 
                             unsigned dpl, unsigned ist)
{
    gate->offset_low    = (uint16_t)func;
    gate->bits.p        = 1;
    gate->bits.dpl      = dpl;
    gate->bits.zero     = 0;
    gate->bits.type     = type;
    gate->bits.reserved = 0;
    gate->offset_middle = (uint16_t) (func >> 16);
    gate->segment       = __KERNEL_CS;
    gate->bits.ist      = ist;
    gate->reserved_zero = 0;
    gate->offset_high   = (uint32_t) (func >> 32);

}

#define load_idt(dtr) native_load_idt(dtr)
static inline void native_load_idt(const struct desc_ptr *dtr)
{
    __asm__ volatile ("lidt %0"::"m" (*dtr));
}

extern void *isr_stub_table[256];

void init_idt(void);

#endif 