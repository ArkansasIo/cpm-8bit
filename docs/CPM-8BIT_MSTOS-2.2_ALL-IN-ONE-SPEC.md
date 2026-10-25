# CPM-8BIT / MSTOS 2.2 — All-in-One System Specification

**Project:** CPM-8BIT
**System:** MSTOS 2.2
**Status:** Architecture and implementation master specification
**Code policy:** All new implementation code, interfaces, schemas, tests, and tooling are to be independently authored for this project.

## 1. Purpose

CPM-8BIT is an Osborne-inspired modern computer platform. The first implementation runs as a Windows application hosting a virtual computer. The architecture is designed to migrate later to dedicated ARM64 or RISC-V hardware.

The project preserves the CP/M/Osborne identity while using a modern 64-bit execution environment, Linux kernel, MSTOS 2.2 system personality, C++23 APIs, and a controlled Z80/CP/M compatibility subsystem.

### Design goals

- Independently authored and unique project implementation.
- Explicit, versioned virtual-hardware interfaces.
- Real resource reporting: configured guest resources in the emulator and firmware-discovered resources on physical hardware.
- Clear separation between Windows host resources and guest resources.
- CP/M/Osborne compatibility through a Z80 emulation layer.
- One architecture that can evolve from Windows VM to physical hardware.

## 2. Master Architecture

```text
Windows
  ↓
CPM-8BIT Windows Application
  ↓
Virtual CPU / RAM / Storage / Display / Input / Network / Audio
  ↓
Firmware / POST
  ↓
Boot Manager
  ↓
Linux Kernel + initramfs
  ↓
MSTOS 2.2 Services
  ↓
C++23 System API
  ↓
MSTOS Desktop / Applications
```

Legacy compatibility path:

```text
CP/M Application
  ↓
Z80 Emulator
  ↓
CP/M BIOS/BDOS Compatibility
  ↓
Virtual Osborne Storage and I/O
```

A pure Windows process does not directly become a Linux kernel. The Windows product therefore hosts a real guest kernel through the project's VM/emulation backend.

## 3. Machine Profiles

### Osborne Compatibility Profile

- Z80-compatible CPU model.
- 64 KiB logical address space.
- CP/M-compatible boot environment.
- Osborne disk-image support.
- Character-oriented display and keyboard/console I/O.

### Modern-64 Profile

- 64-bit guest CPU.
- Configurable RAM.
- Block storage.
- Framebuffer/display device.
- Keyboard and pointer input.
- Network adapter.
- Optional audio.
- Interrupt controller.
- Timers and RTC.
- Modern boot support.

### Physical-64 Profile

Future ARM64 or RISC-V implementation using the same architectural contracts wherever practical.

## 4. CPU and Execution

The execution subsystem provides:

- Z80 backend for CP/M/Osborne compatibility.
- 64-bit backend for the modern guest.
- CPU-state serialization for snapshots.
- Deterministic execution mode.
- Instruction tracing.
- Breakpoints and watchpoints.
- Single-step execution.
- Disassembly/debug inspection.
- CPU feature identification exposed to firmware and MSTOS.

## 5. Memory Architecture

Modern-64 guest RAM is dynamically configurable. The emulator reports the configured guest RAM, not total Windows host RAM. Host memory must never be presented as guest memory without explicit labeling.

| Region | Purpose | Policy |
|---|---|---|
| `0x0000_0000–0x000F_FFFF` | Firmware/low-memory compatibility region | Profile dependent |
| `0x0010_0000–guest RAM` | General guest RAM | Configurable |
| MMIO region | Virtual devices | Defined by I/O specification |
| Reserved regions | Firmware/device/reserved areas | Not general RAM |

The final address map must be versioned in `MEMORY_MAP.md` and tested by automated conformance tests.

## 6. I/O and Virtual Devices

The platform defines explicit contracts for:

- Interrupt controller.
- Timer and monotonic clock.
- RTC.
- Storage controller.
- Display/framebuffer controller.
- Keyboard controller.
- Pointer input.
- Serial/console device.
- Virtual Ethernet adapter.
- Optional audio adapter.
- Debug/diagnostic device.

Every device requires a unique project identifier, register/MMIO contract, reset state, interrupt behavior, error model, version field, and deterministic tests.

## 7. Firmware and POST

The boot screen must report actual discovered/configured values.

Example:

```text
OSBORNE COMPUTER SYSTEM / MSTOS 2.2
CPU ............ OK
MEMORY ......... <actual guest/physical amount> OK
STORAGE ........ <detected capacity> OK
DISPLAY ........ OK
KEYBOARD ....... OK
NETWORK ........ OK / DISABLED
BOOT DEVICE .... <selected device>
```

POST records machine identity, firmware version, hardware/device inventory, detected memory, storage, failures, and diagnostic codes.

## 8. Boot Manager

Required boot entries:

1. Normal MSTOS boot.
2. Linux/recovery boot.
3. CP/M disk boot.
4. Diagnostics.
5. Safe/recovery mode.

The boot manager also provides configurable boot order, timeout, boot arguments, integrity verification, and safe fallback behavior.

## 9. Linux Integration

Linux is the underlying kernel/platform layer. MSTOS 2.2 is a distinct system personality and service layer above Linux rather than a replacement kernel.

Required integration work:

- Reproducible kernel builds.
- initramfs.
- Root filesystem.
- Virtual device support.
- Kernel command line.
- Kernel logging.
- Crash/recovery handling.
- ARM64 target.
- RISC-V target.
- Development x86-64 target where useful.

## 10. MSTOS 2.2 Core

Core services include:

- System initialization.
- Service manager.
- Process and thread management.
- Filesystem/storage services.
- Device manager.
- User/session manager.
- IPC.
- Permissions.
- Configuration manager.
- System information.
- Logging.
- Timers.
- Recovery services.

## 11. C++23 System API

The public API is original, versioned, documented, and tested.

Planned namespaces/services:

```text
MSTOS::System
MSTOS::Process
MSTOS::Thread
MSTOS::Memory
MSTOS::File
MSTOS::Directory
MSTOS::Device
MSTOS::Window
MSTOS::Graphics
MSTOS::Input
MSTOS::Audio
MSTOS::Network
MSTOS::Timer
MSTOS::IPC
MSTOS::Security
```

The API must define ownership, error handling, ABI/version policy, threading rules, permissions, and compatibility guarantees.

## 12. Desktop and GUI

The Osborne-inspired desktop includes:

- Desktop shell.
- Window manager/compositor.
- Application launcher.
- Terminal.
- File manager.
- Settings.
- System monitor.
- Hardware monitor.
- Storage manager.
- Network manager.
- Shutdown/reboot/recovery controls.

The GUI must operate within the guest security model and must not receive unrestricted Windows host access.

## 13. CP/M and Osborne Compatibility

Required compatibility components:

- Z80 CPU emulator.
- CP/M BIOS/BDOS compatibility layer.
- Console I/O.
- Virtual drives A–D.
- Disk-image support.
- Osborne disk preservation/import tools.
- Legacy application launcher.
- CP/M terminal.
- Controlled host/guest file transfer.

Historical CP/M binaries and other third-party software must be distributed only according to their applicable licenses and redistribution rights.

## 14. Storage and Disk Images

Storage specifications include:

- Versioned virtual disk-image format.
- Bootable MSTOS images.
- CP/M/Osborne disk images.
- Mount/unmount operations.
- Snapshots and rollback.
- Integrity hashes.
- Backup/restore.
- Recovery/read-only mode.
- Filesystem corruption detection.

## 15. Configuration

Machine profiles define CPU, RAM, storage, display, input, network, audio, boot order, compatibility mode, and security policy.

Configuration files must be versioned and validated. Malformed or unsupported configuration must fail safely with an actionable diagnostic.

## 16. Snapshots and Save States

A snapshot captures:

- CPU state.
- RAM.
- Device state.
- Storage metadata.
- Timers.
- Machine configuration.
- Snapshot format version.

Snapshots must be validated before restore and must reject incompatible versions safely.

## 17. Networking, Audio, and Clock

Networking supports isolated and host-only modes, with optional bridged networking where explicitly enabled. Guest networking must not imply unrestricted host access.

Audio is an optional virtual device with explicit clock synchronization, mute/disable controls, and deterministic test behavior.

The clock subsystem provides a monotonic timer, guest RTC, timer interrupts, and defined behavior during pause/resume and snapshot restore.

## 18. Security and Sandboxing

Security requirements:

- Guest/host isolation.
- Explicit filesystem sharing.
- No unrestricted registry access.
- No unrestricted host process access.
- Controlled networking.
- Application permissions.
- Signed or hashed system images.
- Secure configuration.
- Recovery controls.
- Optional secure boot for physical hardware.

## 19. Debugging, Logging, and Crash Handling

Developer diagnostics include:

- VM console.
- POST log.
- Boot log.
- Kernel log capture.
- MSTOS service logs.
- CPU instruction trace.
- Register/memory inspection.
- Breakpoints/watchpoints.
- Device-state inspection.
- Crash dumps.
- Reproducible diagnostic bundles.

## 20. Testing and Acceptance

Test categories:

```text
tests/
├── unit/
├── integration/
├── boot/
├── cpu/
├── memory/
├── storage/
├── devices/
├── api/
├── gui/
├── cpm/
├── networking/
├── security/
├── recovery/
└── performance/
```

Primary boot acceptance path:

```text
POST
 ↓
Boot Manager
 ↓
Linux Kernel
 ↓
initramfs
 ↓
MSTOS 2.2
 ↓
C++23 API
 ↓
GUI
```

Every stage must provide a machine-readable success/failure result and useful diagnostic information.

## 21. Build and CI/CD

The project uses a CMake-based C++23 build with architecture-specific Assembly where required.

CI requirements:

- Windows builds.
- Linux builds.
- Unit and integration tests.
- Static analysis.
- Formatting checks.
- Emulator boot tests.
- Compatibility tests.
- Release artifact generation.
- Versioned release notes and checksums.

## 22. Windows Application

The first product is a Windows application hosting the virtual computer. It provides:

- VM lifecycle controls.
- Machine profiles.
- Display/input integration.
- Storage management.
- Logs.
- Debugger access.
- Snapshots.
- Recovery.
- Configuration validation.

## 23. Developer SDK

The SDK contains:

- C++23 headers.
- Libraries.
- Application templates.
- CLI build tools.
- Debugger integration.
- API reference.
- Example applications.
- Version/compatibility policy.

## 24. Performance and Reliability

Performance must be measured rather than assumed. Benchmarks cover:

- Boot time.
- Z80/CP/M throughput.
- Linux startup.
- GUI frame rate.
- Storage throughput.
- Memory usage.
- Host CPU utilization.
- Snapshot creation time.
- Snapshot restore time.
- Long-duration stability.

## 25. Accessibility and Localization

The platform should support:

- Keyboard-first operation.
- Scalable UI.
- High-contrast themes.
- Screen-reader-friendly controls where supported.
- Configurable text sizes.
- Localization-ready resources.
- Unicode in modern MSTOS applications.

## 26. Licensing and Originality

All new CPM-8BIT/MSTOS source code, interfaces, file formats, schemas, tests, and implementation designs should be independently authored for this project.

Third-party components retain their original licenses and attribution. Compatibility with historical systems does not imply ownership of historical software.

The project must not copy source code from other emulators, operating systems, firmware, or proprietary products. Protocols and documented machine behavior may be reimplemented independently for compatibility.

## 27. Physical Hardware Migration

The virtual machine is the reference platform. Future hardware replaces virtual devices with real devices while preserving OS/API contracts where practical.

Target progression:

```text
Windows VM
   ↓
ARM64 / RISC-V development board
   ↓
FPGA / prototype
   ↓
Custom motherboard
   ↓
Osborne-inspired computer
```

## 28. Master Development Roadmap

### Phase 0 — Foundation
Repository standards, architecture documents, C++23/CMake toolchain, Assembly toolchain, licensing policy, and CI foundation.

### Phase 1 — Virtual Machine Specification
CPU, RAM, memory map, I/O map, interrupts, timers, storage, display, keyboard, network, audio, configuration, and deterministic device contracts.

### Phase 2 — Windows Emulator
VM lifecycle, CPU execution, RAM, MMIO, virtual devices, storage, display, keyboard/mouse, networking, audio, snapshots, and debugger.

### Phase 3 — Firmware and POST
Firmware initialization, memory discovery/testing, storage detection, display/keyboard/network initialization, inventory, POST errors, and diagnostic logging.

### Phase 4 — Boot Manager
MSTOS, Linux/recovery, CP/M, diagnostics, safe mode, boot order, timeout, arguments, integrity checking, and fallback.

### Phase 5 — Linux Integration
Kernel build, initramfs, root filesystem, virtual hardware support, kernel command line, logging, crash handling, ARM64/RISC-V support.

### Phase 6 — MSTOS 2.2 Core
Initialization, services, processes, threads, filesystem, devices, users, IPC, permissions, configuration, logging, timers, and recovery.

### Phase 7 — C++23 API
Public headers, ABI/versioning, system calls/services, error model, threading rules, security model, and conformance tests.

### Phase 8 — Osborne Desktop
Desktop shell, windows, launcher, terminal, file manager, settings, hardware monitor, storage/network management, and recovery UI.

### Phase 9 — CP/M Compatibility
Z80 CPU, BIOS/BDOS layer, drives, disk images, legacy launcher, terminal, preservation/import tools, and controlled file transfer.

### Phase 10 — Developer SDK
Headers, libraries, templates, build tools, debugger integration, documentation, examples, and compatibility policy.

### Phase 11 — Storage
Disk-image implementation, filesystem support, snapshots, rollback, integrity, backup, restore, and recovery.

### Phase 12 — Networking/Audio/Clock
Virtual networking, TCP/IP integration, audio device, timers, RTC, pause/resume semantics, and snapshot clock semantics.

### Phase 13 — Security
Sandboxing, permissions, image verification, secure configuration, recovery security, and optional physical secure boot.

### Phase 14 — Testing
Automated unit, integration, boot, CPU, memory, storage, device, API, GUI, CP/M, networking, security, recovery, and performance testing.

### Phase 15 — Release System
Windows installer, portable build, VM images, version manifests, updates, rollback, checksums, release notes, and CI artifacts.

### Phase 16 — Performance/Reliability
Benchmarks, profiling, memory/CPU targets, long-duration tests, watchdogs, crash recovery, and regression tracking.

### Phase 17 — Hardware Prototype
Move the reference virtual architecture to ARM64/RISC-V development hardware and validate real firmware/resource discovery.

### Phase 18 — Physical Osborne-Inspired Computer
SoC, RAM, storage, display, keyboard, USB, networking, audio, RTC, power management, firmware, enclosure, and hardware validation.

### Phase 19 — Compatibility and Preservation
CP/M software preservation, Osborne disk conversion, historical profiles, compatibility tests, and documentation.

### Phase 20 — MSTOS 2.2 Release 1.0
Release the stable Windows VM, MSTOS platform, C++23 SDK/API, CP/M compatibility environment, documentation, and reproducible build artifacts.

## 29. Planned Documentation Set

The repository should maintain focused specifications including:

- `ARCHITECTURE.md`
- `MEMORY_MAP.md`
- `IO_MAP.md`
- `VIRTUAL_MACHINE_SPEC.md`
- `CPU_SPEC.md`
- `STORAGE_SPEC.md`
- `DISPLAY_SPEC.md`
- `KEYBOARD_SPEC.md`
- `NETWORK_SPEC.md`
- `AUDIO_SPEC.md`
- `CLOCK_TIMER_SPEC.md`
- `BOOT_SPEC.md`
- `POST_SPEC.md`
- `LINUX_INTEGRATION.md`
- `MSTOS_SPEC.md`
- `API_SPEC.md`
- `GUI_SPEC.md`
- `CPM_COMPATIBILITY.md`
- `DISK_IMAGE_SPEC.md`
- `SNAPSHOT_SPEC.md`
- `CONFIG_SPEC.md`
- `SECURITY.md`
- `TESTING.md`
- `DEBUGGING.md`
- `PERFORMANCE.md`
- `BUILD.md`
- `CI_CD.md`
- `RELEASE.md`
- `SDK.md`
- `HARDWARE_MIGRATION.md`
- `PHYSICAL_HARDWARE.md`

## 30. Definition of Done

The first complete platform milestone is reached when:

- A reproducible Windows build launches the virtual computer.
- POST reports actual configured guest resources.
- Boot Manager selects and verifies a boot target.
- A real Linux kernel boots inside the VM.
- MSTOS 2.2 services initialize above Linux.
- The C++23 API is versioned and tested.
- The desktop operates without unrestricted host access.
- CP/M software executes through the Z80 compatibility layer.
- Snapshots save, restore, and validate correctly.
- CI verifies the boot and compatibility path.
- The specifications can guide future physical hardware.

## 31. Unique-Code Requirement

CPM-8BIT/MSTOS implementation code must be independently authored for this project. Project namespaces, modules, interfaces, device contracts, test suites, configuration schemas, and tools should be designed specifically for CPM-8BIT/MSTOS.

Compatibility layers may implement documented external protocols or historical machine behavior, but must not copy source code from existing implementations.

## 32. Reference System Flow

```text
Windows
  ↓
CPM-8BIT VM
  ↓
Virtual CPU / RAM / Devices
  ↓
Firmware
  ↓
POST
  ↓
Boot Manager
  ↓
Linux Kernel
  ↓
MSTOS 2.2
  ↓
C++23 API
  ↓
Desktop / Services
  ↓
Applications

Parallel compatibility path:
CP/M → Z80 Emulator → CP/M BIOS/BDOS → Osborne Disk Images
```

## 33. Engineering Principle

**The specification is the contract.** The emulator, firmware, Linux integration, MSTOS services, C++23 API, GUI, CP/M compatibility layer, tests, and future physical hardware must implement the same versioned architecture rather than becoming unrelated implementations.
