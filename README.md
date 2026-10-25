# TinyOS-8 — C++23 Object-Oriented Kernel with MS-DOS 2.2 API (8-bit hardware)

A small educational operating system inspired by the **Linux kernel layout**
(boot → setup → kernel), written in **x86 Assembly** and **C++23** using an
**object-oriented** design, exposing an **MS-DOS 2.2 style API** (INT 21h
function layer) over **8-bit hardware** (8-bit ports, 8259 PIC, 8253 PIT,
8-bit VGA text attributes, PS/2 keyboard scancodes).

## Architecture

```
boot/boot.asm          512-byte bootsector (loads kernel from disk)
kernel/linker.ld       links kernel at physical address 0x7E00
include/               C++23 headers (Kernel, Hardware, Dos subsystems)
src/*.cpp              implementation
src/interrupts.asm     interrupt stubs (INT 21h software API + IRQs)
```

### C++23 OOP design

- `hw::Port8`        — RAII 8-bit I/O port abstraction
- `hw::Device`       — abstract base device, `hw::Pic`, `hw::Pit`, `hw::Vga`, `hw::Keyboard`
- `hw::Bus`          — device registry (polymorphic container)
- `dos::DosApi`      — MS-DOS 2.2 function layer (INT 21h): AH = function, DL/DH = 8-bit args
- `dos::FileHandle`  — file control block object (FCB, DOS 2.2 style)
- `kernel::Kernel`   — singleton orchestrator (boot → init → idle loop)

### MS-DOS 2.2 functions emulated (AH codes)

| AH | Meaning              | Status |
|----|----------------------|--------|
| 01h| Read char w/ echo    | ✅ |
| 02h| Write char (DL)      | ✅ |
| 09h| Write $-string       | ✅ |
| 0Ah| Buffered input       | ✅ |
| 25h| Set interrupt vector | ✅ |
| 2Ch| Get time (CH:CL:DH)  | ✅ |
| 35h| Get interrupt vector | ✅ |
| 4Ch| Terminate (exit code)| ✅ |

## Building

Requirements on the build host: `nasm`, `g++` (C++23), `ld`, `qemu-system-i386`.

```sh
make            # builds kernel.bin
make floppy     # builds floppy.img (bootsector + kernel)
make run        # boots it in QEMU
make asm        # disassembly for review
```

## Notes

- "8-bit" here means 8-bit data paths and registers (`inb/outb`, 8259/8253,
  8-bit DOS register args), true to the era's hardware — the x86 CPU itself
  still runs in 16-bit real mode.
- Kernel entry is `_kstart` (assembly) which calls `kernel::Kernel::instance().run()`.
