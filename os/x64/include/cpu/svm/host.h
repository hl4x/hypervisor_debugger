#ifndef __HOST_H
#define __HOST_H

#include <stdbool.h>
#include <ptrcheck.h>

#include "mm/paging.h"

struct svm_host
{
    void *__sized_by(4096) hsave;
    uint64_t hsave_pa;

    void *__sized_by(4096) vmsave;
    uint64_t vmsave_pa;

    void *__sized_by(4096 * 3) iopm;
    uint64_t iopm_pa;

    void *__sized_by(4096 * 2) msrpm;
    uint64_t msrpm_pa;

    uint32_t max_asid;
    bool has_nrips;
    bool has_np;
} __attribute__((packed));
typedef struct svm_host svm_host_t;

#endif // __HOST_H
