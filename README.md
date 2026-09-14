# hypervisor_debugger

Bare-metal x86-64 kernel for bringing up an AMD SVM (SVM) hypervisor from scratch.
Boots via Multiboot + GRUB, enters long mode, then works toward `VMRUN` / `VMEXIT` handling.

## Status

Done:

- 32-bit boot → long mode, GDT + 64-bit TSS
- IDT + exception handlers
- Physical bump allocator from the Multiboot memory map

Next (see `NOTES.md`):

- SVM detection / enable, VMCB + host-save area, minimal guest, `VMRUN`, `VMEXIT` handling, NPT

## Requirements

- AMD CPU with SVM + KVM enabled (`Makefile` runs QEMU with `-cpu host -enable-kvm`)
- Linux — tested on:
  - AMD Ryzen 5 2600
  - `Linux phanteks-desktop 7.0.0-28-generic #28~24.04.1-Ubuntu SMP PREEMPT_DYNAMIC x86_64 GNU/Linux`
- Packages:
  ```sh
  sudo apt install grub-pc-bin qemu-system-x86 nasm gdb git cmake ninja-build
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
make          # build .output/hypervisor_debugger.iso
make run      # QEMU with KVM, serial to stdio (-nographic)
make debug    # QEMU with -s -S (waits for GDB on :1234)
make clean
```

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
  x64/src/boot/     # Multiboot, serial, page tables, long-mode entry
  x64/src/cpu/      # TSS, SVM (stub)
  x64/src/irq/      # IDT + ISRs
  x64/src/mm/       # Multiboot parsing, bump allocator
  x64/src/util/     # printf, panic
```
