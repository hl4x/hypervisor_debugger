; see https://wiki.osdev.org/Bare_Bones_with_NASM

; Declare constants for the multiboot header.
MBALIGN  equ  1 << 0            ; align loaded modules on page boundaries
MEMINFO  equ  1 << 1            ; provide memory map
MBFLAGS  equ  MBALIGN | MEMINFO ; this is the Multiboot 'flag' field
MAGIC    equ  0x1BADB002        ; 'magic number' lets bootloader find the header
CHECKSUM equ -(MAGIC + MBFLAGS) ; checksum of above, to prove we are multiboot
                                ; CHECKSUM + MAGIC + MBFLAGS should be Zero (0)

section .multiboot
align 4
	dd MAGIC
	dd MBFLAGS
	dd CHECKSUM

extern kernel32_reserved_start
extern kernel64_reserved_start
extern start64_linear
extern stack64_top
extern stack_top
extern pml4_base
extern pdpe_base
extern pde_base
extern pte_base
extern main

; Data start
section .data
; Access bits
PRESENT        equ 1 << 7
NOT_SYS        equ 1 << 4
EXEC           equ 1 << 3
DC             equ 1 << 2
RW             equ 1 << 1
ACCESSED       equ 1 << 0

; Flags bits
GRAN_4K       equ 1 << 7
SZ_32         equ 1 << 6
LONG_MODE     equ 1 << 5

BOOT_INIT_PGT_SIZE equ 4 * 4096

; Text start
section .text
global _start
_start:
	mov esp, stack_top
    push ebx
	call main 
loop:
    jmp loop

global outb
outb:
    mov dx, [esp+4]
    mov al, [esp+8]
    out dx, al
    ret

global inb
inb:
    mov dx, [esp+4]
    in al, dx 
    ret

global setup_pgtable_32
setup_pgtable_32:
    ; essentially memset(pgtable, 0, BOOT_INIT_PGT_SIZE)
    mov edi, pml4_base
    xor eax, eax
    mov ecx, BOOT_INIT_PGT_SIZE/4
    rep stosd

    ; construct pml4 
    mov edi, pml4_base
    lea eax, [pdpe_base + 0x1003] ; 0x1000 | 0x0003 for present and w/r bits
    mov dword [edi], eax

    ; construct pdpe
    mov edi, pdpe_base
    lea eax, [pde_base + 0x1003]
    mov dword [edi], eax

    ; construct pde
    mov edi, pde_base
    lea eax, [pte_base + 0x1003]
    mov dword [edi], eax

    ; construct pte
    mov edi, pte_base
    lea eax, [kernel32_reserved_start + 0x1003]
    mov dword [edi], eax

global enter_long_mode
enter_long_mode:
    ; disable paging (CR0.PG=0)
    mov eax, cr0
    btr eax, 31 ; CR0.PG is bit 31 
    mov cr0, eax

    ; enable PAE
    mov eax, cr4
    bts eax, 5 ; CR4.PAE is bit 5
    mov cr4, eax

    ; load CR3 with the physical base-address of pml4
    mov eax, pml4_base
    mov cr3, eax

    ; enable long mode (EFER.LME=1)
    mov ecx, 0c0000080h ; EFER MSR number
    rdmsr
    bts eax, 8          ; set LME=1
    wrmsr

    ; enable paging to activate long mode (CR0.PG=1)
    mov eax, cr0
    bts eax, 31
    mov cr0, eax

    lgdt [GDT64.Pointer] ; load the 64-but global descriptor table

    db 0eah ; far jump so CS.L=1
    dd start64_linear
    dw 8    ; offset into .Code GDT64 entry

section .code64
bits 64

global start64 
start64:
    mov rsp, stack64_top
    lidt [IDT64.Pointer]
    mov ax, GDT64.TSS
    ltr ax
    mov rcx, 0xfaded
    hlt

section .gdt
GDT64: 
    .Null: equ $ - GDT64
        dq 0
    .Code: equ $ - GDT64
        dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
        db 0                                        ; Base (mid, bits 16-23)
        db PRESENT | NOT_SYS | EXEC | RW            ; Access
        db GRAN_4K | LONG_MODE | 0xF                ; Flags & Limit (high, bits 16-19)
        db 0                                        ; Base (high, bits 24-31)
    .Data: equ $ - GDT64
        dd 0xFFFF                                   ; Limit & Base (low, bits 0-15)
        db 0                                        ; Base (mid, bits 16-23)
        db PRESENT | NOT_SYS | RW                   ; Access
        db GRAN_4K | SZ_32 | 0xF                    ; Flags & Limit (high, bits 16-19)
        db 0                                        ; Base (high, bits 24-31)
    .TSS: equ $ - GDT64
        dd 0x00000068
        dd 0x00CF8900
    .Pointer:
        dw $ - GDT64 - 1
        dq GDT64

section .idt 
IDT64:
    .Pointer:
        dw $ - IDT64 - 1
        dq IDT64

section .pgtable
