#ifndef __IDT_VECTORS_H
#define __IDT_VECTORS_H

#define IDT_DIVIDE_ERROR                 0   /* #DE */
#define IDT_DEBUG                        1   /* #DB */
#define IDT_NMI_INTERRUPT                2   /* #NMI */
#define IDT_BREAKPOINT                   3   /* #BP */
#define IDT_OVERFLOW                     4   /* #OF */
#define IDT_BOUND_RANGE_EXCEEDED         5   /* #BR */
#define IDT_INVALID_OPCODE               6   /* #UD */
#define IDT_DEVICE_NOT_AVAILABLE         7   /* #NM */
#define IDT_DOUBLE_FAULT                 8   /* #DF (Pushes Error Code) */
#define IDT_COPROCESSOR_SEGMENT_OVERRUN  9   /* Reserved */
#define IDT_INVALID_TSS                  10  /* #TS (Pushes Error Code) */
#define IDT_SEGMENT_NOT_PRESENT          11  /* #NP (Pushes Error Code) */
#define IDT_STACK_SEGMENT_FAULT          12  /* #SS (Pushes Error Code) */
#define IDT_GENERAL_PROTECTION_FAULT     13  /* #GP (Pushes Error Code) */
#define IDT_PAGE_FAULT                   14  /* #PF (Pushes Error Code) */
#define IDT_RESERVED_15                  15  /* Reserved */
#define IDT_X87_FLOATING_POINT_ERROR     16  /* #MF */
#define IDT_ALIGNMENT_CHECK              17  /* #AC (Pushes Error Code) */
#define IDT_MACHINE_CHECK                18  /* #MC */
#define IDT_SIMD_FLOATING_POINT_ERROR    19  /* #XF */
#define IDT_RESERVED_20                  20  /* Reserved in AMD64 (#VE in Intel) */
#define IDT_CONTROL_PROTECTION_EXCEPTION 21  /* #CP (Pushes Error Code) */
#define IDT_RESERVED_22_27_MIN           22  /* Reserved range (22-27) */
#define IDT_RESERVED_22_27_MAX           27  
#define IDT_HYPERVISOR_INJECTION         28  /* #HV (AMD-specific) */
#define IDT_VMM_COMMUNICATION            29  /* #VC (AMD SEV - Pushes Error Code) */
#define IDT_SECURITY_EXCEPTION           30  /* #SX (AMD Security - Pushes Error Code) */
#define IDT_RESERVED_31                  31  /* Reserved */

#endif // __IDT_VECTORS_H