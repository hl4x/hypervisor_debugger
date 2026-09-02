#include <stdint.h>

#include "cpu/tss.h"

static struct tss64 tss = {0};

// refernce GDT from boot.s
extern uint8_t GDT[];
extern uint8_t stack64_top[];

static uint8_t df_stack[DOUBLE_FAULT_STACK_SIZE];

void init_tss()
{
    struct tss_descriptor *tss_desc = (struct tss_descriptor*)&GDT[TSS_SEGMENT_SELECTOR];
    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(struct tss64) - 1;

    tss.rsp[0] = (uint64_t)stack64_top;
    tss.ist[0] = (uint64_t)df_stack + sizeof(df_stack);
    tss.iopb_offset = sizeof(struct tss64);

    tss_desc->limit0 = (uint16_t)(tss_limit & 0xFFFF);
    tss_desc->base0 = (uint16_t)(tss_base & 0xFFFF);

    tss_desc->bits.base1 = (uint8_t) ((tss_base >> 16) & 0xFF);
    tss_desc->bits.type = 0x9; // Available 64-bit TSS
    tss_desc->bits.zero = 0; 
    tss_desc->bits.dpl = 0;
    tss_desc->bits.p    = 1; // segment present
    tss_desc->bits.limit1 = (uint8_t)((tss_limit >> 16) & 0x0F);
    tss_desc->bits.avl = 0;
    tss_desc->bits.reserved0 = 0;
    tss_desc->bits.g = 0;
    tss_desc->bits.base2 = (uint8_t)((tss_base >> 24) & 0xFF);

    tss_desc->base3 = (uint32_t)((tss_base >> 32) & 0xFFFFFFFF);
    tss_desc->reserved1 = 0;

    __asm__ volatile ("ltr %0" : : "r" ((uint16_t)TSS_SEGMENT_SELECTOR) : "memory");
}