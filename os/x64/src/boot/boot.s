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

extern kernel_main

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

KERNEL_LOAD_ADDRESS equ 0x00100000

PORT equ 0x3f8 ; COM1

IDT64:
    .Pointer:
        dw $ - IDT64 - 1
        dq IDT64

; Text start
section .text
bits 64
global _start 
_start:
    mov rsp, stack64_top
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang

; inb and outb for printf
global outb
outb:
    mov dx, di ; PORT
    mov al, sil ; VAL
    out dx, al
    ret

global inb
inb:
    mov dx, di ; PORT
    in al, dx
    ret

section .boot.text
bits 32

global start32
start32:
	mov esp, stack32_top
    call init_serial
    call setup_pgtable
    call enter_long_mode
.hang:
    cli
    hlt 
    jmp .hang

global init_serial
init_serial:
    ;
    ; see: https://wiki.osdev.org/Serial_Ports
    ;
    mov dx, PORT + 1 ; DX = destination port
    xor al, al       ; AL = value
    out dx, al 

    mov dx, PORT + 3
    mov al, 0x80
    out dx, al

    mov dx, PORT + 0
    mov al, 0x03
    out dx, al

    mov dx, PORT + 1
    xor al, al
    out dx, al

    mov dx, PORT + 3
    mov al, 0x03
    out dx, al

    mov dx, PORT + 2
    mov al, 0xC7
    out dx, al

    mov dx, PORT + 4
    mov al, 0x0B
    out dx, al

    mov dx, PORT + 4
    mov al, 0x1E
    out dx, al

    mov dx, PORT + 0
    mov al, 0xAE
    out dx, al

    ; set serial in normal operation mode
    mov dx, PORT + 4
    mov al, 0x0F
    out dx, al

    ret

global setup_pgtable
setup_pgtable:
    ;
    ; Identity map the first 2 MiB of physicial memory
    ;
    xor edi, edi
    ; essentially memset(pgtable, 0, BOOT_INIT_PGT_SIZE)
    mov edi, pml4_base
    xor eax, eax
    mov ecx, BOOT_INIT_PGT_SIZE / 4
    rep stosd

    ; construct pml4 
    ; PML4[0] -> PDPE
    mov edi, pml4_base
    mov eax, pdpe_base + 3 ; 3 (0b11) for Present and W/R page-translation bits
    mov dword [edi], eax

    ; construct pdpe
    ; PDPE[0] -> PDE
    mov edi, pdpe_base
    mov eax, pde_base + 3
    mov dword [edi], eax

    ; construct pde
    ; PDE[0] -> PTE
    mov edi, pde_base
    mov eax, pte_base + 3
    mov dword [edi], eax

    ; construct and populate PTEs
    ; kernel starts at 0x00100000 (1 MiB) from the linker script
    mov edi, pte_base
    mov eax, 3 ; Present and W/R bits
    mov ecx, 512
.map:
    mov dword [edi], eax
    add edi, 8
    add eax, 0x1000
    loop .map

    ret

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
    dd _start 
    dw 8    ; offset into .Code GDT64 entry

section .rodata
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
        dd GDT64


section .bss
align 4096

; Reserve 4 KiB for each page table
pml4_base: resb 4096
pdpe_base: resb 4096
pde_base:  resb 4096
pte_base:  resb 4096

; Reserve 16 KiB for the 32-bit stack
stack32_bottom: resb 16384
stack32_top:

; Reserve 16 KiB for the 64-bit stack
stack64_bottom: resb 16384
stack64_top:


