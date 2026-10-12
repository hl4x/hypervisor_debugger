#ifndef __SVM_H
#define __SVM_H

#include "cpu/svm/host.h"
#include "cpu/svm/vcpu.h"

#define MSR_EFER 0xC0000080U
#define MSR_VM_HSAVE_PA 0xC0010117U

// IA32_CR_PAT reset value (APM Vol 2): WB,WT,UC-,UC,WB,WT,UC-,UC
#define MSR_IA32_CR_PAT       0x00000277U
#define SVM_GUEST_PAT_RESET   0x0007040600070406ULL

#define CR0_PE (1ULL << 0)
#define CR0_PG (1ULL << 31)

#define CR4_PAE (1ULL << 5)

#define SVM_GUEST_DR7 0x400
#define SVM_GUEST_DR6 0xFFFF0FF0

#define EFER_LME (1ULL << 8)
#define EFER_LMA (1ULL << 10)
#define EFER_SVME (1ULL << 12)

// bit 1 is hardwired to 1; bit 9 = IF (so hlt can wake on interrupts)
#define SVM_RFLAGS_RESERVED1 (1u << 1)
#define SVM_RFLAGS_IF (1u << 9)
#define SVM_GUEST_RFLAGS (SVM_RFLAGS_RESERVED1 | SVM_RFLAGS_IF)

// APM Vol.2 Table 15-20
#define VMEXIT_EXCP 0x40
#define VMEXIT_HLT 0x78
#define VMEXIT_INTR 0x60
#define VMEXIT_NMI 0x61
#define VMEXIT_SHUTDOWN 0x7f
#define VMEXIT_VMRUN 0x80
#define VMEXIT_INVALID -1

// cpuid function numbers
#define CPUID_MAX_FN 0x00000000U

// cpuid extended function numbers
#define CPUID_EXT_MAX_FN 0x80000000U
#define CPUID_EXT_FEATURES 0x80000001U
#define CPUID_EXT_SVM_FEATURES 0x8000000aU

#define CPUID_NO_OPT_INPUT 0U

// Fn8000_0001_ECX[SVM]
#define CPUID_EXT_FEATURES_ECX_SVM (1U << 2)
// Fn8000_000a_EDX[SVML]
#define CPUID_EXT_SVM_FEATURES_EDX_SVML (1U << 2)

extern void gdt;

enum svm_status {
    SVM_NOT_AVAIL,
    SVM_ALLOWED,
    SVM_DISABLED_AT_BIOS_NOT_UNLOCKABLE,
    SVM_DISABLED_WITH_KEY,
    SVM_NOT_AMD,
};
typedef enum svm_status svm_status_t;

svm_status_t svm_get_status(void);
const char *svm_status_str(svm_status_t s);
void svm_host_init(svm_host_t *host);
void svm_enable(svm_host_t *host);
void svm_query_features(svm_host_t *host);
void svm_vcpu_init(vcpu_t *vcpu, svm_host_t *host,
                   uint64_t entry_rip, uint64_t stack_top,
                   uint64_t cr3, bool has_nrips);
void svm_vmrun(vcpu_t *vcpu);
void svm_dump_exit(vcpu_t *vcpu);
void svm_advance_rip(vcpu_t *vcpu);
void svm_handle_exit(vcpu_t *vcpu);

#endif // __SVM_H
