use a bump allocator to start with, no need to free text or data ever or can't even
-   gonna need a real allocator at some point so can transition to that at some point
-   don't need a coalescing allocator such as gnu style

\
For GDT: \
Create a real 64-bit TSS and load TR with ltr


1. Done :D  Clean up GDT 
2. Done :D  Basic physical memory allocator
3. Done :D  IDT + exception handlers 
4. Done :D  SVM capability detection
5. Done :D  Enable SVM
6. Done :D  Allocate VMCB + host-save area
7. Done :D  Build minimal VMCB
8. Done :D  Build a tiny guest
9. Done :D  VMRUN
10. Done :D  Handle VMEXIT
11. Done :D  Print why you exited
12. Populate IOPM/MSRPM + useful intercepts (CPUID, MSR, IOIO)
13. Guest paging / NPT

Current gaps:
- guest.bin is passed as a grub module; guest runs on host CR3 (shared page tables)
- IOPM/MSRPM are allocated + zeroed => no IO/MSR intercepts yet
- vmsave buffer allocated but unused (no VMSAVE/VMVLOAD yet)
- single vcpu, no event injection, no ASID/TLB management