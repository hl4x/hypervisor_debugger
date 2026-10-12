#ifndef __SVM_H
#define __SVM_H

#include "cpu/svm/host.h"
#include "cpu/svm/vcpu.h"

#define MSR_EFER 0xC0000080U
#define MSR_VM_HSAVE_PA 0xC0010117U

#define EFER_SVME (1ULL << 12)
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

#endif // __SVM_H
