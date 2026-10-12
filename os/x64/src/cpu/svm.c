#include <stdint.h>
#include <stdbool.h>

#include "mm/mm.h"
#include "cpu/cpu.h"
#include "cpu/msr.h"
#include "util/bug.h"
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
    vcpu->vmcb_pa = (uint64_t)vcpu->vmcb;

    vcpu->vmcb->control.intercepts[INTERCEPT_EXCEPTION] = (1u << 3);
    vcpu->vmcb->control.intercepts[INTERCEPT_WORD3] =
        (1u << (INTERCEPT_INTR - 96)) | // bit 0
        (1u << (INTERCEPT_NMI - 96)) | // bit 1
        (1u << (INTERCEPT_HLT - 96)); // bit 24
    vcpu->vmcb->control.intercepts[INTERCEPT_WORD4] =
        (1u << (INTERCEPT_VMRUN - 128)); // bit 0
    vcpu->vmcb->control.asid_tlb_ctl = (1ULL << 32) | 1;
    vcpu->vmcb->control.iopm_base_pa = host->iopm_pa;
    vcpu->vmcb->control.msrpm_base_pa = host->msrpm_pa;
    vcpu->vmcb->control.v_tpr_v_irq_intctl = (1ULL << 24);
    vcpu->vmcb->control.nested_ctl = 0;
    vcpu->vmcb->control.event_inj = 0;
    vcpu->vmcb->control.event_inj_err = 0;

    // todo: add save as well
    //vcpu->vmcb->save.cs.selector =

    guest_regs_store(&vcpu->guest_regs);

    vcpu->entry_rip = entry_rip;
    vcpu->stack_top = stack_top;
    vcpu->cr3 = cr3;
    vcpu->has_nrips = has_nrips;
}
