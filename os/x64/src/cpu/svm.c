#include <stdint.h>
#include <stdbool.h>

#include "mm/mm.h"
#include "util/bug.h"
#include "util/printf.h"
#include "irq/idt.h"
#include "cpu/cpu.h"
#include "cpu/msr.h"
#include "cpu/tss.h"
#include "cpu/svm/svm.h"
#include "cpu/svm/vmcb.h"
#include "cpu/svm/host.h"
#include "cpu/svm/vcpu.h"


const char *svm_status_str(svm_status_t s)
{
    // place table itself in .rodata as well
    static const char *const names[] = {
        [SVM_NOT_AVAIL] = "SVM_NOT_AVAIL",
        [SVM_ALLOWED] = "SVM_ALLOWED",
        [SVM_DISABLED_AT_BIOS_NOT_UNLOCKABLE] = "SVM_DISABLED_AT_BIOS_NOT_UNLOCKABLE",
        [SVM_DISABLED_WITH_KEY] = "SVM_DISABLED_WITH_KEY",
        [SVM_NOT_AMD] = "SVM_NOT_AMD"
    };
    if ((int)s < 0 || (unsigned)s >= sizeof(names)/sizeof(names[0])) return "UNKNOWN";
    return names[s];
}

// see 15.4 Enabling SVM
svm_status_t svm_get_status(void)
{
    uint32_t a, b, c, d, ignored;

    // Check if we're running on an AMD cpu
    cpuid(CPUID_MAX_FN, CPUID_NO_OPT_INPUT,
          &a, &b, &c, &d);
    if ((b != 0x68747541U)    || // htuA
           (c != 0x444D4163U) || // DMAc
           (d != 0x69746E65U))   // itne
        return SVM_NOT_AMD;

    cpuid(CPUID_EXT_MAX_FN, CPUID_NO_OPT_INPUT,
          &a, &ignored, &ignored, &ignored);
    if (a < CPUID_EXT_SVM_FEATURES)
        return SVM_NOT_AVAIL;

    cpuid(CPUID_EXT_FEATURES, CPUID_NO_OPT_INPUT,
          &ignored, &ignored, &c, &ignored);
    if (!(c & CPUID_EXT_FEATURES_ECX_SVM))
        return SVM_NOT_AVAIL;

    uint64_t vmcr = rdmsr(MSR_VM_CR);
    if ((vmcr & MSR_VM_CR_SVMDIS) == 0)
        return SVM_ALLOWED;

    cpuid(CPUID_EXT_SVM_FEATURES, CPUID_NO_OPT_INPUT,
          &ignored, &ignored, &ignored, &d);
    if (!(d & CPUID_EXT_SVM_FEATURES_EDX_SVML))
        return SVM_DISABLED_AT_BIOS_NOT_UNLOCKABLE;
    else return SVM_DISABLED_WITH_KEY;
}

void svm_host_init(svm_host_t *host)
{
    // caller is responsible for allocating hsave, vmsave, iopm, and msrpm
    memset(host->hsave, 0, FOUR_K);
    host->hsave_pa = (uint64_t)host->hsave;

    memset(host->vmsave, 0, FOUR_K);
    host->vmsave_pa = (uint64_t)host->vmsave;

    memset(host->iopm, 0, FOUR_K * 3);
    host->iopm_pa = (uint64_t)host->iopm;

    memset(host->msrpm, 0, FOUR_K * 2);
    host->msrpm_pa = (uint64_t)host->msrpm;

    svm_query_features(host);
}

void svm_query_features(svm_host_t *host)
{
    uint32_t a, b, c, d;
    cpuid(CPUID_EXT_SVM_FEATURES, 0, &a, &b, &c, &d);

    host->max_asid = b & 0xFF; // EBX[7:0]
    if (host->max_asid == 0)
        host->max_asid = 1; // fallback

    host->has_nrips = (d >> 3) & 1; // EDX[3]
    host->has_np = d & 1; // EDX[0]
}

void svm_enable(svm_host_t *host)
{
    uint64_t status = svm_get_status();
    BUG_ON(status != SVM_ALLOWED,
           "SVM not usable: %s (%d)\n",
           svm_status_str(status), status);

    // set EFER.SVME = 1
    uint64_t efer = rdmsr(MSR_EFER);
    efer |= EFER_SVME;
    wrmsr(MSR_EFER, efer);

    // set VM_HSAVE_PA MSR
    BUG_ON(!host->hsave, "svm_enable: host hsave not set");
    wrmsr(MSR_VM_HSAVE_PA, host->hsave_pa);
}

void svm_vcpu_init(vcpu_t *vcpu, svm_host_t *host,
                   uint64_t entry_rip, uint64_t stack_top,
                   uint64_t cr3, bool has_nrips)
{
    BUG_ON(!vcpu, "svm_vcpu_init: vcpu not set");
    memset(vcpu, 0, sizeof(vcpu_t));

    vcpu->vmcb = bump_alloc_aligned(FOUR_K, FOUR_K);
    memset(vcpu->vmcb, 0, FOUR_K);
    vcpu->vmcb_pa = (uint64_t)vcpu->vmcb;

    vmcb_control_area_t *control = &vcpu->vmcb->control;
    vmcb_save_area_t *save = &vcpu->vmcb->save;

    control->intercepts[INTERCEPT_EXCEPTION] = (1u << 3);
    control->intercepts[INTERCEPT_WORD3] =
        (1u << (INTERCEPT_INTR - 96)) | // bit 0
        (1u << (INTERCEPT_NMI - 96)) | // bit 1
        (1u << (INTERCEPT_HLT - 96)); // bit 24
    control->intercepts[INTERCEPT_WORD4] =
        (1u << (INTERCEPT_VMRUN - 128)); // bit 0
    control->asid_tlb_ctl = (1ULL << 32) | 1;
    control->iopm_base_pa = host->iopm_pa;
    control->msrpm_base_pa = host->msrpm_pa;
    control->v_tpr_v_irq_intctl = 0;
    control->nested_ctl = 0;
    control->event_inj = 0;
    control->event_inj_err = 0;

    pack_vmcb_seg(&save->cs, 0x08,
                  0xa9b, 0xffffffff, 0);
    pack_vmcb_seg(&save->ss, 0x10,
                  0x893, 0xffffffff, 0);
    pack_vmcb_seg(&save->ds, 0x0,
                  0x893, 0xffffffff, 0);
    pack_vmcb_seg(&save->es, 0x0,
                  0x893, 0xffffffff, 0);
    pack_vmcb_seg(&save->fs, 0x0,
                  0x893, 0xffffffff, 0);
    pack_vmcb_seg(&save->gs, 0x0,
                  0x893, 0xffffffff, 0);
    save->gdtr.limit = 0x27;
    save->gdtr.base = (uint64_t)&GDT;
    save->idtr.limit = idtr.size;
    save->idtr.base = idtr.address;
    pack_vmcb_seg(&save->ldtr, 0, 0, 0, 0);
    pack_vmcb_seg(&save->tr, 0x18, 0x8b,
                  tss_limit, tss_base);
    save->cpl = 0;
    save->efer = EFER_LME | EFER_LMA| EFER_SVME;
    save->cr0 = CR0_PE | CR0_PG;
    save->cr4 = CR4_PAE;
    save->cr3 = cr3;
    save->dr7 = SVM_GUEST_DR7;
    save->dr6 = SVM_GUEST_DR6;
    save->rflags = SVM_GUEST_RFLAGS;
    save->rip = entry_rip;
    save->rsp = stack_top;
    save->star = 0;
    save->lstar = 0;
    save->cstar = 0;
    save->sfmask = 0;
    save->kernel_gs_base = 0;
    save->sysenter_cs = 0;
    save->sysenter_esp = 0;
    save->sysenter_eip= 0;
    save->cr2 = 0;
    save->g_pat = SVM_GUEST_PAT_RESET;

    vcpu->entry_rip = entry_rip;
    vcpu->stack_top = stack_top;
    vcpu->cr3 = cr3;
    vcpu->has_nrips = has_nrips;
}

void svm_dump_exit(vcpu_t *vcpu)
{
    BUG_ON(!vcpu || !vcpu->vmcb, "svm_dump_exit: no vcpu/vmcb");
    vmcb_control_area_t *c = &vcpu->vmcb->control;
    vmcb_save_area_t *s = &vcpu->vmcb->save;
    guest_regs_t *g = &vcpu->guest_regs;

    printf("VMEXIT code 0x%lx info1 0x%lx info2 0x%lx\n",
          (unsigned long)c->exit_code,
          (unsigned long)c->exit_info_1,
          (unsigned long)c->exit_info_2);
    printf("\tRIP 0x%lx RSP 0x%lx RFLAGS 0x%lx next_RIP 0x%lx len %u\n",
           (unsigned long)s->rip, (unsigned long)s->rsp,
           (unsigned long)s->rflags, (unsigned long)c->next_rip,
           c->insn_len);
    printf("\tVMCB_RAX 0x%lx\n", (unsigned long)s->rax);
    printf("\tr15 %lx r14 %lx r13 %lx r12 %lx\n",
           (unsigned long)g->r15, (unsigned long)g->r14,
           (unsigned long)g->r13, (unsigned long)g->r12);
    printf("\tr11 %lx r10 %lx r9 %lx r8 %lx\n",
           (unsigned long)g->r11, (unsigned long)g->r10,
           (unsigned long)g->r9, (unsigned long)g->r8);
    printf("\trbp %lx rdi %lx rsi %lx rdx %lx rcx %lx rbx %lx\n",
           (unsigned long)g->rbp, (unsigned long)g->rdi, (unsigned long)g->rsi,
           (unsigned long)g->rdx, (unsigned long)g->rcx, (unsigned long)g->rbx);
}

void svm_advance_rip(vcpu_t *vcpu)
{
    BUG_ON(!vcpu || !vcpu->vmcb, "svm_advance_rip: no vcpu/vmcb");
    vmcb_control_area_t *control = &vcpu->vmcb->control;
    vmcb_save_area_t *save = &vcpu->vmcb->save;

    if (vcpu->has_nrips) {
        save->rip = control->next_rip;
        // force cpu to reload modified rip
        control->vmcb_clean = 0;
    }
    else {
        BUG_ON(control->insn_len == 0,
               "infinite loop detected, 0 len instruction");
        save->rip += control->insn_len;
        control->vmcb_clean = 0;
    }
}

void svm_handle_exit(vcpu_t *vcpu)
{
    BUG_ON(!vcpu || !vcpu->vmcb, "svm_handle_exit: no vcpu/vmcb");
    vmcb_control_area_t *control = &vcpu->vmcb->control;
    uint64_t exit_code = control->exit_code;

    if (exit_code >= 0x40  && exit_code <= 0x5f) {
        // this should be the test int3
        unsigned long vec = exit_code - 0x40;
        BUG_ON(vec != 3, "expected #BP(3) got vec 0x%lx", vec);
        svm_dump_exit(vcpu);
        svm_advance_rip(vcpu);
    }
    else switch(exit_code) {
        case VMEXIT_EXCP: {
            unsigned long vec = (unsigned long)(control->exit_info_1 & 0xffULL);
            BUG_ON(vec != 3, "expected #BP(3) got vec 0x%lx", vec);
            svm_advance_rip(vcpu);
            break;
        }
        case VMEXIT_HLT:
            svm_advance_rip(vcpu);
            break;
        case VMEXIT_INTR:
            break;
        case VMEXIT_NMI:
            break;
        case VMEXIT_SHUTDOWN:
            BUG_ON(1, "got VMEXIT_SHUTDOWN");
            break;
        case VMEXIT_INVALID:
        default:
            BUG_ON(1, "unhandled VMEXIT 0x%lx",
                   (unsigned long)control->exit_code);
            break;
    }

    //if (exit_code != VMEXIT_INTR &&
    //    exit_code != VMEXIT_NMI)
    //    svm_dump_exit(vcpu);
}
