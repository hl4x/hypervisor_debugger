#ifndef __VMCB_H
#define __VMCB_H

#include <stddef.h>
#include <stdint.h>

#define VMCB_OFF(type, field, off)              \
    _Static_assert(offsetof(type, field) == (off), #type "." #field " has the wrong offset")

enum intercept_words {
	INTERCEPT_CR = 0,
	INTERCEPT_DR,
	INTERCEPT_EXCEPTION,
	INTERCEPT_WORD3,
	INTERCEPT_WORD4,
	INTERCEPT_WORD5,
	MAX_INTERCEPT,
};

enum {
	/* Byte offset 000h (word 0) */
	INTERCEPT_CR0_READ = 0,
	INTERCEPT_CR3_READ = 3,
	INTERCEPT_CR4_READ = 4,
	INTERCEPT_CR8_READ = 8,
	INTERCEPT_CR0_WRITE = 16,
	INTERCEPT_CR3_WRITE = 16 + 3,
	INTERCEPT_CR4_WRITE = 16 + 4,
	INTERCEPT_CR8_WRITE = 16 + 8,
	/* Byte offset 004h (word 1) */
	INTERCEPT_DR0_READ = 32,
	INTERCEPT_DR1_READ,
	INTERCEPT_DR2_READ,
	INTERCEPT_DR3_READ,
	INTERCEPT_DR4_READ,
	INTERCEPT_DR5_READ,
	INTERCEPT_DR6_READ,
	INTERCEPT_DR7_READ,
	INTERCEPT_DR0_WRITE = 48,
	INTERCEPT_DR1_WRITE,
	INTERCEPT_DR2_WRITE,
	INTERCEPT_DR3_WRITE,
	INTERCEPT_DR4_WRITE,
	INTERCEPT_DR5_WRITE,
	INTERCEPT_DR6_WRITE,
	INTERCEPT_DR7_WRITE,
	/* Byte offset 008h (word 2) */
	INTERCEPT_EXCEPTION_OFFSET = 64,
	/* Byte offset 00Ch (word 3) */
	INTERCEPT_INTR = 96,
	INTERCEPT_NMI,
	INTERCEPT_SMI,
	INTERCEPT_INIT,
	INTERCEPT_VINTR,
	INTERCEPT_SELECTIVE_CR0,
	INTERCEPT_STORE_IDTR,
	INTERCEPT_STORE_GDTR,
	INTERCEPT_STORE_LDTR,
	INTERCEPT_STORE_TR,
	INTERCEPT_LOAD_IDTR,
	INTERCEPT_LOAD_GDTR,
	INTERCEPT_LOAD_LDTR,
	INTERCEPT_LOAD_TR,
	INTERCEPT_RDTSC,
	INTERCEPT_RDPMC,
	INTERCEPT_PUSHF,
	INTERCEPT_POPF,
	INTERCEPT_CPUID,
	INTERCEPT_RSM,
	INTERCEPT_IRET,
	INTERCEPT_INTn,
	INTERCEPT_INVD,
	INTERCEPT_PAUSE,
	INTERCEPT_HLT,
	INTERCEPT_INVLPG,
	INTERCEPT_INVLPGA,
	INTERCEPT_IOIO_PROT,
	INTERCEPT_MSR_PROT,
	INTERCEPT_TASK_SWITCH,
	INTERCEPT_FERR_FREEZE,
	INTERCEPT_SHUTDOWN,
	/* Byte offset 010h (word 4) */
	INTERCEPT_VMRUN = 128,
	INTERCEPT_VMMCALL,
	INTERCEPT_VMLOAD,
	INTERCEPT_VMSAVE,
	INTERCEPT_STGI,
	INTERCEPT_CLGI,
	INTERCEPT_SKINIT,
	INTERCEPT_RDTSCP,
	INTERCEPT_ICEBP,
	INTERCEPT_WBINVD,
	INTERCEPT_MONITOR,
	INTERCEPT_MWAIT,
	INTERCEPT_MWAIT_COND,
	INTERCEPT_XSETBV,
	INTERCEPT_RDPRU,
	TRAP_EFER_WRITE,
	TRAP_CR0_WRITE,
	TRAP_CR1_WRITE,
	TRAP_CR2_WRITE,
	TRAP_CR3_WRITE,
	TRAP_CR4_WRITE,
	TRAP_CR5_WRITE,
	TRAP_CR6_WRITE,
	TRAP_CR7_WRITE,
	TRAP_CR8_WRITE,
	/* Byte offset 014h (word 5) */
	INTERCEPT_INVLPGB = 160,
	INTERCEPT_INVLPGB_ILLEGAL,
	INTERCEPT_INVPCID,
	INTERCEPT_MCOMMIT,
	INTERCEPT_TLBSYNC,
};

// See AMD Manual Vol 2: Appendix B VMCB Layout
struct vmcb_control_area
{
    uint32_t intercepts[MAX_INTERCEPT];         // 000h
    uint32_t reserved_018h[15 - MAX_INTERCEPT]; // 018h
    uint16_t pause_filter_thresh;               // 03Ch
    uint16_t pause_filter_count;                // 03Eh
    uint64_t iopm_base_pa;                      // 040h
    uint64_t msrpm_base_pa;                     // 048h
    uint64_t tsc_offset;                        // 050h
    uint64_t asid_tlb_ctl;                      // 058h
    uint64_t v_tpr_v_irq_intctl;                // 060h
    uint64_t interrupt_shadow;                  // 068h
    uint64_t exit_code;                         // 070h
    uint64_t exit_info_1;                       // 078h
    uint64_t exit_info_2;                       // 080h
    uint64_t exit_int_info;                     // 088h
    uint64_t nested_ctl;                        // 090h
    uint8_t reserved_098h[16];                  // 098h
    uint32_t event_inj;                         // 0A8h
    uint32_t event_inj_err;
    uint64_t nested_cr3;                        // 0B0h
    uint64_t reserved_0B8h;                     // 0B8h
    uint32_t vmcb_clean;                        // 0C0h
    uint32_t reserved_0C4h;
    uint64_t next_rip;                          // 0C8h
    uint8_t insn_len;                           // 0D0h
    uint8_t insn_bytes[15];
    uint8_t reserved_0E0h[0x320];               // 0E0h

} __attribute__((packed));
typedef struct vmcb_control_area vmcb_control_area_t;
_Static_assert(sizeof(vmcb_control_area_t) == 0x400, "vmcb_control_area is not 0x400 bytes large");

struct vmcb_seg
{
    uint16_t selector;
    uint16_t attrib;
    uint32_t limit;
    uint64_t base;
} __attribute__((packed));
typedef struct vmcb_seg vmcb_seg_t;
_Static_assert(sizeof(vmcb_seg_t) == 0x10, "vmcb_seg not sized correct");

static inline
void pack_vmcb_seg(vmcb_seg_t *seg, uint16_t selector,
                   uint16_t attrib, uint32_t limit,
                   uint64_t base)
{
    seg->selector = selector;
    seg->attrib = attrib;
    seg->limit = limit;
    seg->base = base;
}

struct vmcb_save_area
{
    vmcb_seg_t es, cs, ss, ds, fs, gs, gdtr, ldtr, idtr, tr;
    uint8_t reserved_0A0h[43];
    uint8_t cpl;
    uint32_t reserved_0CCh;
    uint64_t efer;
    uint8_t reserved_0D8h[112];
    uint64_t cr4;
    uint64_t cr3;
    uint64_t cr0;
    uint64_t dr7;
    uint64_t dr6;
    uint64_t rflags;
    uint64_t rip;
    uint8_t reserved_180h[88];
    uint64_t rsp;
    uint64_t s_cet;
    uint64_t ssp;
    uint64_t isst_addr;
    uint64_t rax;
    uint64_t star;
    uint64_t lstar;
    uint64_t cstar;
    uint64_t sfmask;
    uint64_t kernel_gs_base;
    uint64_t sysenter_cs;
    uint64_t sysenter_esp;
    uint64_t sysenter_eip;
    uint64_t cr2;
    uint8_t reserved_0248h[32];
    uint64_t g_pat;
} __attribute__((packed));
typedef struct vmcb_save_area vmcb_save_area_t;
_Static_assert(sizeof(vmcb_save_area_t) < (FOUR_K - sizeof(vmcb_control_area_t)), "vmcb_save_area too large!");

struct vmcb
{
    vmcb_control_area_t control;

    // note, no SEV-ES VMs for now
    vmcb_save_area_t save;

    // pad to 4096 bytes
    uint8_t reserved_0670h[4096 - sizeof(vmcb_control_area_t) - sizeof(vmcb_save_area_t)];
} __attribute__((packed));
typedef struct vmcb vmcb_t;
_Static_assert(sizeof(vmcb_t) == 4096, "vmcb must be 4096 bytes (one page)");
VMCB_OFF(vmcb_t, save, 0x400);

VMCB_OFF(vmcb_save_area_t, es, 0x0);
VMCB_OFF(vmcb_save_area_t, cs, 0x10);
VMCB_OFF(vmcb_save_area_t, ss, 0x20);
VMCB_OFF(vmcb_save_area_t, ds, 0x30);
VMCB_OFF(vmcb_save_area_t, fs, 0x40);
VMCB_OFF(vmcb_save_area_t, gs, 0x50);
VMCB_OFF(vmcb_save_area_t, gdtr, 0x60);
VMCB_OFF(vmcb_save_area_t, ldtr, 0x70);
VMCB_OFF(vmcb_save_area_t, idtr, 0x80);
VMCB_OFF(vmcb_save_area_t, tr, 0x90);
VMCB_OFF(vmcb_save_area_t, cpl, 0xcb);
VMCB_OFF(vmcb_save_area_t, efer, 0xd0);
VMCB_OFF(vmcb_save_area_t, cr4, 0x148);
VMCB_OFF(vmcb_save_area_t, cr3, 0x150);
VMCB_OFF(vmcb_save_area_t, cr0, 0x158);
VMCB_OFF(vmcb_save_area_t, dr7, 0x160);
VMCB_OFF(vmcb_save_area_t, dr6, 0x168);
VMCB_OFF(vmcb_save_area_t, rflags, 0x170);
VMCB_OFF(vmcb_save_area_t, rip, 0x178);
VMCB_OFF(vmcb_save_area_t, rsp, 0x1d8);
VMCB_OFF(vmcb_save_area_t, s_cet, 0x1e0);
VMCB_OFF(vmcb_save_area_t, ssp, 0x1e8);
VMCB_OFF(vmcb_save_area_t, isst_addr, 0x1f0);
VMCB_OFF(vmcb_save_area_t, rax, 0x1f8);
VMCB_OFF(vmcb_save_area_t, star, 0x200);
VMCB_OFF(vmcb_save_area_t, lstar, 0x208);
VMCB_OFF(vmcb_save_area_t, cstar, 0x210);
VMCB_OFF(vmcb_save_area_t, sfmask, 0x218);
VMCB_OFF(vmcb_save_area_t, kernel_gs_base, 0x220);
VMCB_OFF(vmcb_save_area_t, sysenter_cs, 0x228);
VMCB_OFF(vmcb_save_area_t, sysenter_esp, 0x230);
VMCB_OFF(vmcb_save_area_t, sysenter_eip, 0x238);
VMCB_OFF(vmcb_save_area_t, cr2, 0x240);
VMCB_OFF(vmcb_save_area_t, g_pat, 0x268);

VMCB_OFF(vmcb_control_area_t, intercepts, 0);
VMCB_OFF(vmcb_control_area_t, pause_filter_thresh, 0x3c);
VMCB_OFF(vmcb_control_area_t, pause_filter_count, 0x3e);
VMCB_OFF(vmcb_control_area_t, iopm_base_pa, 0x40);
VMCB_OFF(vmcb_control_area_t, msrpm_base_pa, 0x48);
VMCB_OFF(vmcb_control_area_t, tsc_offset, 0x50);
VMCB_OFF(vmcb_control_area_t, asid_tlb_ctl, 0x58);
VMCB_OFF(vmcb_control_area_t, v_tpr_v_irq_intctl, 0x60);
VMCB_OFF(vmcb_control_area_t, interrupt_shadow, 0x68);
VMCB_OFF(vmcb_control_area_t, exit_code, 0x70);
VMCB_OFF(vmcb_control_area_t, exit_info_1, 0x78);
VMCB_OFF(vmcb_control_area_t, exit_info_2, 0x80);
VMCB_OFF(vmcb_control_area_t, exit_int_info, 0x88);
VMCB_OFF(vmcb_control_area_t, nested_ctl, 0x90);
VMCB_OFF(vmcb_control_area_t, event_inj, 0xa8);
VMCB_OFF(vmcb_control_area_t, event_inj_err, 0xac);
VMCB_OFF(vmcb_control_area_t, nested_cr3, 0xb0);
VMCB_OFF(vmcb_control_area_t, vmcb_clean, 0xc0);
VMCB_OFF(vmcb_control_area_t, next_rip, 0xc8);
VMCB_OFF(vmcb_control_area_t, insn_len, 0xd0);
VMCB_OFF(vmcb_control_area_t, insn_bytes, 0xd1);

#endif // __VMCB_H
