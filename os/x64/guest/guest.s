section .text
guest_entry:
    int3
.hlt_loop:
    hlt
    jmp .hlt_loop
