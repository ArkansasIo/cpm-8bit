# Osborne MSTOS 2.2

A modern Osborne 1-inspired computer and operating environment, **built first
as a Windows application** (virtual Osborne hardware), with the proven
architecture later portable to ARM64/RISC-V physical hardware.

```
Osborne 1 heritage → MSTOS 2.2 environment → Modern 64-bit Linux platform
```

## Current status

| Phase | Description                              | Status |
|-------|------------------------------------------|--------|
| 1     | Windows app with virtual Osborne hardware | ✅ M1 |
| 2     | Functional boot sequence (config-driven)  | ✅ M2–3 |
| 3     | Real POST diagnostics over virtual HW     | ✅ |
| 4     | Osborne Boot Manager                      | ✅ |
| 5     | Linux kernel integration (simulated)      | 🚧 M4 |
| 6     | MSTOS 2.2 services + shell                | ✅ M5–6 |
| 7     | Osborne-inspired GUI                      | 🚧 M7 |
| 8     | CP/M Z80 compatibility layer              | 🚧 M8 |
| 9     | Recovery environment                      | 🚧 M9 |
| 10    | ARM64/RISC-V physical port                | ⏳ M10 |

## Build (Windows)

```powershell
powershell -File scripts\build.ps1        # builds build\mstos.exe
build\mstos.exe                           # boots the virtual Osborne
build\mstos.exe --ram 32768 --disk 1024   # override virtual hardware
```

Requirements: CMake ≥ 3.20 and a C++23 compiler (MSVC 19.3x+ or MinGW g++ 13+).

## Try it

The virtual machine POSTs using values from **real configured hardware**
(nothing is hard-coded), shows the boot manager, then hands off to the
simulated 64-bit Linux kernel load and starts the MSTOS 2.2 environment:

```
OSBORNE COMPUTER SYSTEM
MSTOS 2.2

CPU ............... OK (x86-64, 8 cores)
MEMORY ............ 16384 MB OK
STORAGE ........... 512 GB OK
VIDEO ............. OK (80x25 + graphics)
KEYBOARD .......... OK
NETWORK ........... OK

Linux Kernel ...... LOADING
Initramfs ......... LOADING
MSTOS 2.2 ......... STARTING

A:\> ver
```

Boot manager: `1` MSTOS, `2` Recovery, `3` Diagnostics, `4` CP/M Disk,
`5` Boot device selection, `ESC` return.
