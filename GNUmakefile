.SUFFIXES:

LLVM_PREFIX := $(shell brew --prefix llvm)
CXX := $(LLVM_PREFIX)/bin/clang++
LD  := $(shell command -v ld.lld)

CXXFLAGS := \
  --target=x86_64-unknown-elf \
  -ffreestanding -fno-stack-protector -fno-stack-check \
  -fno-exceptions -fno-rtti -fno-threadsafe-statics \
  -fno-pic -fno-pie \
  -m64 -march=x86-64 -mabi=sysv -mcmodel=kernel \
  -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -mno-80387 \
  -Wall -Wextra -O2 -g \
  -ffunction-sections -fdata-sections

LDFLAGS := \
  -nostdlib -static \
  -T linker.ld \
  --gc-sections \
  -z max-page-size=0x1000 \
  -Map kernel.map

KERNEL_OBJS := main.o mem.o

all: kernel.elf image.iso

%.o: kernel/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

kernel.elf: $(KERNEL_OBJS) linker.ld
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o $@

image.iso: kernel.elf limine.conf
	rm -rf iso_root
	mkdir -p iso_root/boot/limine iso_root/EFI/BOOT iso_root/boot
	cp -v kernel.elf iso_root/boot/kernel.elf
	cp -v limine.conf iso_root/boot/limine/limine.conf
	cp -v limine/limine-bios.sys limine/limine-bios-cd.bin limine/limine-uefi-cd.bin iso_root/boot/limine/
	cp -v limine/BOOTX64.EFI iso_root/EFI/BOOT/
	cp -v limine/BOOTIA32.EFI iso_root/EFI/BOOT/ 2>/dev/null || true
	xorriso -as mkisofs -R -r -J -b boot/limine/limine-bios-cd.bin \
	  -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
	  -apm-block-size 2048 --efi-boot boot/limine/limine-uefi-cd.bin \
	  -efi-boot-part --efi-boot-image --protective-msdos-label \
	  iso_root -o image.iso
	./limine/limine bios-install image.iso

run: image.iso
	qemu-system-x86_64 -m 1024 -cdrom image.iso \
	  -debugcon stdio -serial null \
	  -no-reboot -no-shutdown

clean:
	rm -f *.o kernel.elf kernel.map image.iso
	rm -rf iso_root
