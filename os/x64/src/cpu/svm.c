#include <stdint.h>
#include <stdbool.h>

#include "cpu/svm.h"
#include "cpu/cpu.h"
#include "util/bug.h"
#include "util/printf.h"

bool is_svm_supported()
{
    uint32_t a, b, c, d, ignored;

    cpuid(0x00000000, 0, &a, &b, &c, &d);
    BUG_ON((b != 0x68747541U) || // htuA
           (c != 0x444D4163U) || // DMAc
           (d != 0x69746E65U),   // itne
           "Not authentic AMD cpu");
    printf("on amd cpu :^)");
}
