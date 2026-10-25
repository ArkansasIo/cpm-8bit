# Makefile — TinyOS-8
NASM   ?= nasm
CXX    ?= g++
LD     ?= ld
QEMU   ?= qemu-system-i386

CXXFLAGS = -std=c++23 -ffreestanding -fno-exceptions -fno-rtti -fno-stack-protector \
           -m16 -mgeneral-regs-only -fno-pic -fno-plt -O2 -Wall -Wextra -nostdinc++ \
           -Iinclude
ASFLAGS  = -f elf32
LDFLAGS  = -m elf_i386 -T linker.ld -nostdlib -no-pie

SRCS_CXX = src/kernel.cpp src/vga.cpp src/pic.cpp src/pit.cpp src/keyboard.cpp src/dos_api.cpp
OBJS     = $(SRCS_CXX:.cpp=.o) src/interrupts.o

all: kernel.bin

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.asm
	$(NASM) $(ASFLAGS) $< -o $@

kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel.elf $(OBJS)
	objcopy -O binary kernel.elf kernel.bin

boot/boot.bin: boot/boot.asm
	$(NASM) -f bin $< -o $@

floppy: boot/boot.bin kernel.bin
	powershell -NoProfile -Command "$$img = New-Object byte[] (1440*1024); $$b = [IO.File]::ReadAllBytes('boot/boot.bin'); $$k = [IO.File]::ReadAllBytes('kernel.bin'); $$b.CopyTo($$img,0); $$k.CopyTo($$img,512); [IO.File]::WriteAllBytes('floppy.img',$$img)"

run: floppy
	$(QEMU) -fda floppy.img -boot a -nographic

asm: kernel.bin
	objdump -d -M i8086 kernel.elf > kernel.lst

clean:
	powershell -NoProfile -Command "Get-ChildItem -Recurse -Include *.o,*.bin,*.elf,*.img,*.lst | Remove-Item"

.PHONY: all floppy run asm clean
