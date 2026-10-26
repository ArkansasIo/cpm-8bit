# Makefile — TinyOS-8
NASM   ?= nasm
CXX     = clang++
LD      = ld.lld
QEMU   ?= qemu-system-i386

CXXFLAGS = --target=i386-none-none -m16 -mno-80387 -std=c++23 -ffreestanding -fno-exceptions -fno-rtti \
           -fno-stack-protector -fno-pic -fno-plt -O2 -Wall -Wextra -nostdinc++ \
           -Iinclude
ASFLAGS  = -f elf32
LDFLAGS  = -m elf_i386 -T linker.ld -nostdlib -no-pie

SRCS_CXX = src/kernel.cpp src/vga.cpp src/pic.cpp src/pit.cpp src/keyboard.cpp src/dos_api.cpp src/runtime_stubs.cpp
OBJS     = $(SRCS_CXX:.cpp=.o) src/interrupts.o

all: kernel.bin

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.asm
	$(NASM) $(ASFLAGS) $< -o $@

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel.bin $(OBJS)

boot/boot.bin: boot/boot.asm
	$(NASM) -f bin $< -o $@

floppy: boot/boot.bin kernel.bin
	powershell -NoProfile -File mkfloppy.ps1

run: floppy
	$(QEMU) -fda floppy.img -boot a -nographic

asm: kernel.bin
	objdump -d -b binary -m i8086 kernel.bin > kernel.lst

clean:
	powershell -NoProfile -Command "Get-ChildItem -Recurse -Include *.o,*.bin,*.elf,*.img,*.lst | Remove-Item"

.PHONY: all floppy run asm clean
