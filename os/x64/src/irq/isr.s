section .text
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

extern isr_handler;
isr_common_stub:
    PUSH_ALL
    ; TODO COMPLETE