
%macro PUSH_HOST_REGS 0
    push rbx                    ; high mem
    push rbp
    push r12
    push r13
    push r14
    push r15
    push rdi                    ; low mem
%endmacro

%macro POP_HOST_REGS 0
    pop rdi
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
%endmacro

%macro GUEST_REGS_STORE 0
    mov [rsi + 0], r15
    mov [rsi + 8], r14
    mov [rsi + 16], r13
    mov [rsi + 24], r12
    mov [rsi + 32], r11
    mov [rsi + 40], r10
    mov [rsi + 48], r9
    mov [rsi + 56], r8
    mov [rsi + 64], rbp
    mov [rsi + 72], rdi
    mov [rsi + 88], rdx
    mov [rsi + 96], rcx
    mov [rsi + 104], rbx
    ; store guest rsi from host stack slot
    ; rsp holds guest_RSI from xchg rsi, [rsp]
    ; use rax since it isn't stored in guest regs for now
    mov rax, [rsp]
    mov [rsi + 80], rax
%endmacro

%macro GUEST_REGS_LOAD 0
    mov r15,  [rsi + 0]
    mov r14,  [rsi + 8]
    mov r13, [rsi + 16]
    mov r12, [rsi + 24]
    mov r11, [rsi + 32]
    mov r10, [rsi + 40]
    mov r9, [rsi + 48]
    mov r8, [rsi + 56]
    mov rbp, [rsi + 64]
    mov rdi, [rsi + 72]
    mov rdx, [rsi + 88]
    mov rcx, [rsi + 96]
    mov rbx, [rsi + 104]
    mov rsi, [rsi + 80]
%endmacro

section .text

global svm_vmrun
svm_vmrun:
    mov rax, [rdi + 8]          ; rax = vcpu->vmcb_pa
    lea rsi, [rdi + 16]         ; rsi = &vcpu->guest_regs
    PUSH_HOST_REGS
    push rsi
    cli
    GUEST_REGS_LOAD
    ; rsi holds guest_RSI
    vmrun
; restore rsi --- isn't touched by guest reg stores/load macros
; and vmrun preserves rsp. This also writes the guest rsi back to
; the host stack clear it later
    xchg rsi, [rsp]
    GUEST_REGS_STORE
    add rsp, 8                  ; clear guest rsi on host stack
    POP_HOST_REGS
    sti
    ret
