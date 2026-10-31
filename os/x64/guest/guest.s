section .text
guest_entry:
    ;int3
    ud2
.hlt_loop:
    hlt
    jmp .hlt_loop
