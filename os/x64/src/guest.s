global guest_entry, guest_stack_top
section .text
guest_entry:
    int3
.hlt_loop:
    hlt
    jmp .hlt_loop

section .bss
align 4096
guest_stack_bottom: resb 4096
guest_stack_top:
