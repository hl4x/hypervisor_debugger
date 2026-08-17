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

extern stack_top
extern pml4_base
extern main

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

    lgdt [GDT64.Pointer]
    jmp GDT64.Code:Realm64

Realm64:
    cli                           ; Clear the interrupt flag.
    mov ax, GDT64.Data            ; Set the A-register to the data descriptor.
    mov ds, ax                    ; Set the data segment to the A-register.
    mov es, ax                    ; Set the extra segment to the A-register.
    mov fs, ax                    ; Set the F-segment to the A-register.
    mov gs, ax                    ; Set the G-segment to the A-register.
    mov ss, ax                    ; Set the stack segment to the A-register.
    mov eax, 0x200000             ; load address of x64_kernel into eax
    jmp eax                       ; jmp to x64_kernel

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
