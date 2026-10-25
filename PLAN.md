# Osborne MSTOS 2.2 — Development Plan

## 1. Project Overview

Build a modern **Osborne 1-inspired computer and operating environment** called **MSTOS 2.2**, initially delivered as a Windows application.

The first implementation will run entirely inside Windows so the hardware, boot process, Linux kernel, MSTOS services, GUI, and compatibility layers can be developed and tested safely before moving to dedicated physical hardware.

## 2. Core Architecture

```text
Windows
  │
  ▼
Osborne Emulator / Windows Application
  │
  ├── Virtual CPU
  ├── Virtual RAM
  ├── Virtual Storage
  ├── Virtual Display
  ├── Virtual Keyboard
  └── Virtual Network
  │
  ▼
Osborne Bootloader
  │
  ├── POST diagnostics
  ├── Real virtual-memory detection
  ├── Hardware checks
  └── Boot-device menu
  │
  ▼
64-bit Linux Kernel
  │
  ▼
Initramfs
  │
  ▼
MSTOS 2.2
  │
  ├── System services
  ├── Filesystem
  ├── Device services
  ├── CP/M/Z80 compatibility
  └── C++23 OS API
  │
  ▼
Osborne GUI / Desktop
```

## 3. Development Strategy

### Phase 1 — Windows Application

Create a Windows application that emulates the target machine.

Requirements:

- Run without dedicated hardware.
- Provide configurable virtual RAM.
- Provide configurable virtual storage.
- Provide a virtual display.
- Provide keyboard and mouse input.
- Provide virtual networking where needed.
- Save and load virtual-machine state when practical.

### Phase 2 — Osborne Boot System

Implement an Osborne-inspired boot sequence.

The boot screen should be functional rather than cosmetic.

Example:

```text
OSBORNE COMPUTER SYSTEM
MSTOS 2.2

CPU ............... OK
MEMORY ............ 16384 MB OK
STORAGE ........... 512 GB OK
VIDEO ............. OK
KEYBOARD .......... OK
NETWORK ........... OK

Linux Kernel ...... LOADING
Initramfs ......... LOADING
MSTOS 2.2 ......... STARTING
```

The memory value must come from the configured virtual hardware rather than a hard-coded fake value.

### Phase 3 — POST Diagnostics

Implement actual startup diagnostics for the emulated hardware.

Initial checks:

- CPU
- RAM detection
- RAM test
- Storage
- Display
- Keyboard
- Network

A diagnostic failure should be reported and should prevent normal boot when the failed component is required.

### Phase 4 — Boot Manager

Provide a boot menu such as:

```text
OSBORNE BOOT MANAGER

1. Boot MSTOS 2.2
2. Recovery Mode
3. Diagnostics
4. CP/M Disk
5. Boot Device Selection

ESC - Return
ENTER - Select
```

The boot manager should eventually support multiple boot images and virtual Osborne disk images.

### Phase 5 — Linux

Use a 64-bit Linux kernel as the underlying modern kernel.

Linux is responsible for:

- CPU scheduling
- Memory management
- Processes
- Storage
- Networking
- Device drivers
- Security
- System calls

Avoid modifying/forking Linux unless there is a concrete technical requirement.

### Phase 6 — MSTOS 2.2

MSTOS 2.2 is the Osborne-inspired operating environment above Linux.

Responsibilities:

- System initialization
- Services
- User environment
- Filesystem integration
- CP/M compatibility
- Osborne disk-image management
- Application management
- GUI integration

## 4. CP/M and Osborne Compatibility

Provide a Z80/CP/M compatibility layer so appropriate legacy software can run inside the modern environment.

Expose virtual drives:

```text
A:  Osborne disk image
B:  Osborne disk image
C:  MSTOS filesystem
D:  Modern storage
```

The compatibility layer should be isolated from the Linux kernel and MSTOS core.

## 5. OS API

Create a stable API between MSTOS services and applications.

The primary application/system language should be **C++23**, while low-level hardware and boot components can use Assembly and other appropriate systems languages.

Example API categories:

- Files
- Processes
- Windows
- Graphics
- Input
- Audio
- Networking
- Storage
- System information
- Timers
- IPC

Example:

```cpp
namespace MSTOS {
    File OpenFile(const char* path);
    Process CreateProcess(const char* program);
    Window CreateWindow(int width, int height);
    void DrawText(const char* text);
    void Sleep(uint64_t milliseconds);
}
```

## 6. Programming Languages

### C++23

Primary language for:

- MSTOS services
- GUI
- Application framework
- OS API
- Emulator components where appropriate

### Assembly

Use Assembly only where it provides a real low-level requirement, such as:

- Boot entry
- CPU initialization
- Interrupt entry points
- Context-switch support
- Architecture-specific operations

### C

Use C where Linux/kernel-facing interfaces or low-level components make it appropriate.

### Rust

Rust may be introduced later for selected memory-sensitive or safety-critical components.

## 7. GUI

Create a modern GUI inspired by the Osborne 1 rather than copying its limitations.

The GUI should provide:

- Desktop
- Windows
- Menus
- File manager
- Terminal
- Settings
- System information
- Application launcher
- Boot/recovery interface

The visual design can preserve Osborne-era typography, layout ideas, and character-oriented interaction while supporting modern resolutions.

## 8. Recovery System

Provide a minimal recovery environment that can start when MSTOS or Linux cannot complete normal startup.

Recovery capabilities:

- Hardware diagnostics
- Boot-log inspection
- Filesystem checks
- Kernel selection
- MSTOS recovery
- Configuration repair
- Virtual disk inspection

## 9. Security

Add security progressively.

Planned capabilities:

- Optional verified/secure boot
- Image integrity checks
- Application isolation
- Permission model
- Protected system files
- Signed system components where practical

Security features should not prevent development or emulator testing unnecessarily.

## 10. Future Physical Hardware

After the Windows version is stable, port the architecture to dedicated Osborne-inspired hardware.

Target architecture:

- 64-bit ARM64 or RISC-V initially
- Modern RAM
- Modern persistent storage
- Modern display
- USB
- Network interface
- Keyboard
- Optional original-style enclosure and controls

The physical bootloader should reuse the same conceptual boot sequence but replace virtual hardware detection with real firmware and hardware detection.

## 11. Compatibility Goal

The final system should have three layers of identity:

```text
Osborne 1 heritage
       │
       ▼
MSTOS 2.2 operating environment
       │
       ▼
Modern 64-bit Linux platform
```

The goal is not to turn the original Osborne 1 hardware into a modern Linux computer. The goal is to create a modern system that preserves the Osborne 1's identity and software concepts while providing contemporary computing capabilities.

## 12. Proposed Repository Structure

```text
osborne-mstos/
├── README.md
├── PLAN.md
├── LICENSE
├── docs/
├── boot/
│   ├── boot.asm
│   ├── entry.asm
│   ├── linker.ld
│   ├── boot_config.cfg
│   └── splash/
├── emulator/
│   ├── cpu/
│   ├── memory/
│   ├── storage/
│   ├── display/
│   ├── keyboard/
│   └── network/
├── kernel/
│   └── linux/
├── initramfs/
├── mstos/
│   ├── init/
│   ├── core/
│   ├── services/
│   ├── filesystem/
│   ├── shell/
│   ├── compatibility/
│   └── recovery/
├── api/
│   ├── include/
│   └── src/
├── gui/
│   ├── boot/
│   ├── desktop/
│   ├── windows/
│   ├── widgets/
│   └── compositor/
├── apps/
│   ├── terminal/
│   ├── filemanager/
│   ├── settings/
│   └── systeminfo/
├── cp-m/
│   ├── z80/
│   ├── bios/
│   └── disk-images/
├── hardware/
│   ├── arm64/
│   └── riscv64/
├── scripts/
├── tests/
└── tools/
```

## 13. Milestones

### Milestone 1
Windows application launches with virtual Osborne hardware.

### Milestone 2
Virtual POST detects configured CPU, RAM, storage, display, keyboard, and network.

### Milestone 3
Functional Osborne boot manager.

### Milestone 4
64-bit Linux kernel boots inside the virtual machine.

### Milestone 5
MSTOS 2.2 initializes above Linux.

### Milestone 6
C++23 OS API becomes available.

### Milestone 7
Osborne-inspired GUI launches.

### Milestone 8
CP/M/Z80 compatibility layer runs legacy software.

### Milestone 9
Recovery environment works.

### Milestone 10
System is portable to dedicated ARM64/RISC-V hardware.

## 14. Design Principle

**Build the complete experience in Windows first, then move the proven architecture to physical hardware.**

This minimizes hardware risk while allowing the bootloader, Linux integration, MSTOS 2.2, API, GUI, diagnostics, and compatibility systems to be developed and tested as one coherent project.
