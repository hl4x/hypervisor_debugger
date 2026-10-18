# hypervisor_debugger

Bare-metal x86-64 kernel for bringing up an AMD SVM hypervisor from scratch.
Boots via Multiboot + GRUB, enters long mode, enables SVM, and runs a minimal
guest under `VMRUN` with a `VMEXIT` handler loop.

## Status

Done:

- 32-bit boot → long mode, GDT + 64-bit TSS
- IDT + exception handlers
- Physical bump allocator from the Multiboot memory map
- Identity page tables (4K pages) for kernel/modules/usable RAM + CR3 switch
- Multiboot module parsing (tiny guest binary loaded as a GRUB module)
- SVM detection (CPUID + `VM_CR`) and enable (`EFER.SVME`, `VM_HSAVE_PA`)
- Host-save area / VMCB / IOPM / MSRPM allocation, CPUID SVM feature query
  (max ASID, NRIPS, NP)
- Minimal VMCB + guest (`int3`, then HLT loop), `VMRUN` / `VMEXIT` loop
- Guest general-purpose register save/restore, NRIPS-aware RIP advance
- Basic exit handling + dump: `#BP`, HLT, INTR, NMI, SHUTDOWN, invalid

Next (see `NOTES.md`):

- Populate IOPM/MSRPM + useful intercepts (CPUID, MSR, IOIO)
- Guest paging / NPT
- Event injection, ASID/TLB management

## Requirements

- `make run` needs an AMD CPU with SVM + KVM enabled (`-cpu host -enable-kvm`)
- `make debug` runs under TCG with `-cpu qemu64` (SVM emulated), so no AMD/KVM needed
- Linux — tested on:
  - AMD Ryzen 5 2600
  - `Linux phanteks-desktop 7.0.0-28-generic #28~24.04.1-Ubuntu SMP PREEMPT_DYNAMIC x86_64 GNU/Linux`
- Packages:
  ```sh
  sudo apt install grub-pc-bin qemu-system-x86 nasm gdb git cmake ninja-build mtools
  ```

## Toolchain

Builds a `-fbounds-safety` Clang into `~/toolchain/x86_64-bounds-safety`:

```sh
chmod +x os/scripts/get_and_build_llvm.sh
./os/scripts/get_and_build_llvm.sh
```

## Build / Run / Debug

All commands run from `os/`:

```sh
make          # build guest.bin + .output/hypervisor_debugger.iso
make run      # QEMU with KVM, serial to stdio (-nographic)
make debug    # TCG qemu64 with -s -S (waits for GDB on :1234), log in .output/qemu.log
make clean
```

The ISO's GRUB config passes `guest.bin` as a Multiboot module; the kernel runs it
as the SVM guest.

Attach GDB in another terminal:

```sh
./os/scripts/start_gdb.sh    # or start_gdb.fish
```

QEMU exposes its monitor on `telnet 127.0.0.1:1235`.

## Layout

```text
os/
  Makefile          # kernel -> ISO, run/debug targets
  linker.ld         # kernel loaded at 1 MiB
  scripts/          # LLVM toolchain build, GDB helpers
  x64/guest/        # tiny guest (int3 + hlt loop), built to guest.bin
  x64/src/boot/     # Multiboot, serial, page tables, long-mode entry
  x64/src/cpu/      # TSS, SVM (detection/enable, VMCB, vcpu, vmrun asm)
  x64/src/irq/      # IDT + ISRs
  x64/src/mm/       # Multiboot parsing, bump allocator, paging
  x64/src/util/     # printf, panic
```
