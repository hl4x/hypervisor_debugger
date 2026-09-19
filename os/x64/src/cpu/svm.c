#include <stdint.h>
#include <stdbool.h>

#include "cpu/svm.h"
#include "cpu/cpu.h"
#include "cpu/msr.h"

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
