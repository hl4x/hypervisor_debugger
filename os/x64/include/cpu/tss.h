#ifndef __TSS_H
#define __TSS_H

#define TSS_SEGMENT_SELECTOR 0x18

#define KERNEL_STACK_SIZE       8192
#define DOUBLE_FAULT_STACK_SIZE 4096

struct tss64 {
    uint32_t reserved0;
    uint64_t rsp[3];
    uint64_t reserved1;
    uint64_t ist[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed));

struct tss_bits {
    uint32_t    base1       : 8,
                type        : 4,
                zero        : 1,
                dpl         : 2,
                p           : 1,
                limit1      : 4,
                avl         : 1,
                reserved0   : 2,
                g           : 1,
                base2       : 8;
} __attribute__((packed));

struct tss_descriptor {
    uint16_t limit0;
    uint16_t base0;
    struct tss_bits bits;
    uint32_t base3;
    uint32_t reserved1;
} __attribute__((packed));

void init_tss(void);

#endif // __TSS_H