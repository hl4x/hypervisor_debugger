#ifndef __VCPU_H
#define __VCPU_H

#include <stdint.h>
#include <stdbool.h>

#include "cpu/svm/vmcb.h"

struct guest_regs
{
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx;
};
typedef struct guest_regs guest_regs_t;

struct vcpu
{
    vmcb_t *vmcb;
    uint64_t vmcb_pa;
    guest_regs_t guest_regs;
    uint64_t entry_rip;
    uint64_t stack_top;
    uint64_t cr3;
    bool has_nrips;
};
typedef struct vcpu vcpu_t;
_Static_assert(offsetof(vcpu_t, vmcb_pa) == 8,
    "vmcb_pa misplaced for svm_vmrun");
_Static_assert(offsetof(vcpu_t, guest_regs) == 16,
    "guest_regs misplaced for svm_vmrun");

static inline
void guest_regs_store(guest_regs_t *r)
{
    __asm__ volatile (
        "mov %%rbx, %0\n\t"
        "mov %%rcx, %1\n\t"
        "mov %%rdx, %2\n\t"
        "mov %%rsi, %3\n\t"
        "mov %%rdi, %4\n\t"
        "mov %%rbp, %5\n\t"
        "mov %%r8, %6\n\t"
        "mov %%r9, %7\n\t"
        "mov %%r10, %8\n\t"
        "mov %%r11, %9\n\t"
        "mov %%r12, %10\n\t"
        "mov %%r13, %11\n\t"
        "mov %%r14, %12\n\t"
        "mov %%r15, %13\n\t"
        : "=m"(r->rbx), "=m"(r->rcx), "=m"(r->rdx), "=m"(r->rsi),
          "=m"(r->rdi), "=m"(r->rbp), "=m"(r->r8), "=m"(r->r9),
          "=m"(r->r10), "=m"(r->r11), "=m"(r->r12), "=m"(r->r13),
          "=m"(r->r14), "=m"(r->r15)
        :
        : "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "r8", "r9",
          "r10", "r11", "r12", "r13", "r14", "r15", "memory" );
}

static inline
void guest_regs_load(guest_regs_t *r)
{
    __asm__ volatile (
        "mov %0, %%rbx\n\t"
        "mov %1, %%rcx\n\t"
        "mov %2, %%rdx\n\t"
        "mov %3, %%rsi\n\t"
        "mov %4, %%rdi\n\t"
        "mov %5, %%rbp\n\t"
        "mov %6, %%r8\n\t"
        "mov %7, %%r9\n\t"
        "mov %8, %%r10\n\t"
        "mov %9, %%r11\n\t"
        "mov %10, %%r12\n\t"
        "mov %11, %%r13\n\t"
        "mov %12, %%r14\n\t"
        "mov %13, %%r15\n\t"
        :
        : "m"(r->rbx), "m"(r->rcx), "m"(r->rdx), "m"(r->rsi),
          "m"(r->rdi), "m"(r->rbp), "m"(r->r8), "m"(r->r9),
          "m"(r->r10), "m"(r->r11), "m"(r->r12), "m"(r->r13),
          "m"(r->r14), "m"(r->r15)
        : "rbx", "rcx", "rdx", "rsi", "rdi", "rbp", "r8", "r9",
          "r10", "r11", "r12", "r13", "r14", "r15", "memory" );
}

#endif // __VCPU_H
