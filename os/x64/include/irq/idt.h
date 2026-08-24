#ifndef __IDT_H
#define __IDT_H

#include <stdint.h>

#define IDT_PRESENT
#define TRAP_GATE
#define INTR_GATE

// See figure 4-24 from AMD64 Volume 2 Manual
struct __attribute__((packed)) idt_entry64 {
    uint16_t offset_1; 
    uint16_t target_selector;
    uint8_t ist; // bits 0..2 IST field, bits 3..7 (Reserved, IGN) are ignored in long mode
    uint8_t type;
    uint16_t offset_2;
    uint32_t offset_3;
    uint32_t zero;
};

// See figure 8-14
struct __attribute__((packed)) iret_stack {
    uint64_t old_rip;
    uint64_t old_cs;
    uint64_t old_rflags;
    uint64_t old_rsp;
    uint64_t old_ss;
};

struct __attribute__((packed)) isr_save_state {
    // These need to be defined to match PUSH_ALL and POP_ALL stack layout
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8; 
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax; // rax is high mem and r15 is low (rsp+0) on stack

    uint64_t intr_vector_nr;
    uint64_t err_code;

    struct iret_stack iret;
};

void initialize_idt(void);

#endif 