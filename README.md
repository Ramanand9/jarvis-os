Day 1 Work Log (Boot + Build Bring-Up)
Goal
Create a reproducible build + boot pipeline for a freestanding x86_64 kernel on an M1 Mac, using Limine and QEMU, with deterministic debug output.

1) Tooling setup (macOS)
Installed required tools for kernel build + ISO creation + emulation:
git — version control
llvm (clang/clang++) — cross-compile --target=x86_64-unknown-elf
lld (ld.lld) — ELF linker for kernel
nasm — assembler (needed later)
qemu — run x86_64 VM on Apple Silicon
xorriso — create bootable ISO images
make (GNU make via gmake) — deterministic build recipes
mtools — useful later for disk images

2) Project structure created
Created a minimal repo layout:
kernel/ — kernel source files
limine/ — Limine bootloader repo + binaries
iso_root/ — ISO staging directory (generated)
root files:
GNUmakefile
linker.ld
limine.conf

3) Bootloader setup (Limine)
Cloned Limine repo and built it (gmake -C limine)
Ensured Limine boot files exist for ISO boot:
limine-bios.sys
limine-bios-cd.bin
limine-uefi-cd.bin
BOOTX64.EFI (and BOOTIA32.EFI if present)
Understood Limine’s role:
loads kernel.elf into memory
populates request/response structures
jumps to kernel entry point (kmain)

4) Limine protocol header integrated
Fixed “empty limine.h” by importing the real Limine protocol header
Placed header into kernel/limine.h
Verified header wasn’t empty (line count sanity check)

5) Kernel code (freestanding C++)
Implemented a minimal kernel that boots and proves execution.
kernel/main.cpp
Declared Limine request structures in special ELF sections:
base revision request
framebuffer request
request list start/end markers
Implemented kernel entry:
extern "C" void kmain(void)
Implemented debug output path using QEMU’s port 0xE9:
outb(0xE9, byte) prints to terminal when QEMU runs with -debugcon stdio
Validated Limine responses:
base revision supported
framebuffer response exists
Wrote pixels into framebuffer (diagonal line) as visual proof-of-life
Halted safely with hlt loop (hcf())
kernel/mem.cpp
Implemented minimal libc routines required in freestanding mode:
memcpy, memset, memmove, memcmp
Added minimal C++ ABI stubs:
__cxa_pure_virtual, __cxa_atexit

6) Linker script created (linker.ld)
Defined kernel entry symbol: ENTRY(kmain)
Placed kernel at higher-half virtual address base: 0xffffffff80000000
Ensured Limine request sections are preserved:
KEEP(*(.limine_requests_*))
Defined .text, .rodata, .data, .bss
Discarded unwind/extra metadata (e.g., .eh_frame*) for simplicity

7) Limine configuration created (limine.conf)
Added boot menu entry and kernel path:
Boot entry label: JARVIS-OS
Protocol: limine
Kernel path: boot():/boot/kernel.elf

8) Build system implemented (GNUmakefile)
Created a single-command pipeline:
Compile (freestanding x86_64)
clang++ --target=x86_64-unknown-elf
Freestanding flags:
-ffreestanding
-fno-exceptions -fno-rtti
-mno-red-zone
-mcmodel=kernel
disable SSE/MMX/FPU usage early
Link (ELF kernel)
Used ld.lld discovered via system path (/opt/homebrew/bin/ld.lld)
Linked with -T linker.ld and -nostdlib -static
Produced:
kernel.elf
kernel.map
ISO build
Staged files into iso_root/:
kernel.elf → iso_root/boot/kernel.elf
limine.conf → iso_root/boot/limine/limine.conf
Limine BIOS + UEFI files into correct directories
Created hybrid ISO via xorriso → image.iso
Installed Limine BIOS stages into ISO:
./limine/limine bios-install image.iso
Run target
Booted ISO in QEMU:
qemu-system-x86_64 -cdrom image.iso -debugcon stdio ...

9) Debug + validation (Day 1 success criteria)
Confirmed full bring-up pipeline by observing kernel logs:
[K] kmain entered
[K] Got framebuffer
[K] Drew pixels. Halting.
This proves:
ISO boots
Limine loads kernel
kernel reaches entry point
Limine request/response mechanism works
debug output path is functional

10) Issues solved during Day 1 (lessons learned)
GNU make requires real TABs for recipe lines (not spaces / not literal \t)
Some files got accidentally written with command text inside (fixed by rewriting)
ld.lld was not in $(brew --prefix llvm)/bin; actual location was /opt/homebrew/bin/ld.lld (fixed by using command -v ld.lld style discovery)
Verified build determinism with gmake -f GNUmakefile --debug=v run
