#include "irq/idt_vectors.h"

%macro PUSH_ALL 0
    push rax ; high memory
    push rbx  
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15 ; low memory
%endmacro

%macro POP_ALL 0
    pop r15 ; low memory
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax ; high memory
%endmacro

%macro ISR_NOERRCODE 1
    push 0              ; no err_code
    push %1             ; push intr_vector_nr
    jmp isr_common_stub ; jmp so no return address is pushed to the stack
%endmacro

%macro ISR_ERRCODE 1
    push %1 ; push intr_vector_nr
    jmp isr_common_stub
%endmacro

%define HAS_ERRCODE(i) ( \
    i == IDT_DOUBLE_FAULT || \
    (i >= IDT_INVALID_TSS && i <= IDT_PAGE_FAULT) || \
    i == IDT_ALIGNMENT_CHECK || \
    i == IDT_CONTROL_PROTECTION_EXCEPTION || \
    i == IDT_VMM_COMMUNICATION || \
    i == IDT_SECURITY_EXCEPTION \
)

section .text

extern isr_handler
isr_common_stub:
    PUSH_ALL
    mov rdi, rsp
    mov rbp, rsp    ; save stack
    and rsp, ~0xf
    call isr_handler
    mov rsp, rbp    ; restore stack
    POP_ALL
    add rsp, 0x10   ; skip past err_code and intr_vector_nr 
    iretq

%assign i 0
%rep 256
isr_stub_%[i]:
    %if HAS_ERRCODE(i) 
        ISR_ERRCODE i
    %else
        ISR_NOERRCODE i
    %endif
%assign i i+1
%endrep


section .rodata

global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
    dq isr_stub_%[i] ; place addr of isr stub in r/o mem
%assign i i+1
%endrep