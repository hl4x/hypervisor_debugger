use a bump allocator to start with, no need to free text or data ever or can't even
-   gonna need a real allocator at some point so can transition to that at some point
-   don't need a coalescing allocator such as gnu style

\
For GDT: \
Create a real 64-bit TSS and load TR with ltr


1. Clean up GDT
2. Basic physical memory allocator
3. IDT + exception handlers
4. SVM capability detection
5. Enable SVM
6. Allocate VMCB + host-save area
7. Build minimal VMCB
8. Build a tiny guest
9. VMRUN
10. Handle VMEXIT
11. Print why you exited
12. Add useful intercepts
13. Add guest paging / NPT