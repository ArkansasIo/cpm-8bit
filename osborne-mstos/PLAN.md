# Osborne osbex 2.2 — Development Plan

See `README.md` for status. This file is the authoritative phased plan.

## Phase 1 — Windows Application (virtual Osborne hardware)
Windows application emulating the target machine: configurable virtual RAM,
configurable virtual storage (file-backed disk images), virtual display,
keyboard/mouse input, optional virtual networking, VM state save/load.

## Phase 2 — Osborne Boot System
Functional boot screen. All reported values (CPU, MEMORY, STORAGE, …) come
from the configured virtual hardware, never hard-coded.

## Phase 3 — POST Diagnostics
CPU, RAM detection + RAM test, storage, display, keyboard, network checks.
A failed required component prevents normal boot and reports the failure.

## Phase 4 — Boot Manager
Menu: 1 Boot osbex 2.2 / 2 Recovery Mode / 3 Diagnostics / 4 CP/M Disk /
5 Boot Device Selection. ESC return, ENTER select. Multiple boot images and
virtual Osborne disk images supported.

## Phase 5 — OS Prototype
64-bit OS Prototype kernel underneath: scheduling, memory, processes, storage,
networking, drivers, security, syscalls. Do not fork OS Prototype without a concrete
technical requirement. First milestone: kernel boots inside the virtual
machine (simulated until an image pipeline exists).

## Phase 6 — osbex 2.2
Environment above OS Prototype: init, system services, filesystem integration,
device services, CP/M compatibility, disk-image management, application
management, GUI integration.

## Phase 7 ( Compatibility ) — CP/M & Osborne Compatibility
Z80/CP/M compatibility layer, isolated from OS Prototype and osbex core.
Virtual drives: A:/B: Osborne disk images, C: osbex filesystem, D: modern storage.

## Phase 8 — OS API
Stable C++23 API (`namespace osbex`): Files, Processes, Windows, Graphics,
Input, Audio, Networking, Storage, System info, Timers, IPC.

## Languages
- **C++23** — osbex services, GUI, API, emulator.
- **Assembly** — boot entry, CPU init, interrupt entry, context switch.
- **C** — kernel-facing low-level components.
- **Rust** — later, for memory-sensitive components only.

## GUI
Desktop, windows, menus, file manager, terminal, settings, system info,
launcher, boot/recovery interface. Osborne-era typography and character
interaction at modern resolutions.

## Recovery
Hardware diagnostics, boot-log inspection, fsck, kernel selection, osbex
recovery, configuration repair, virtual disk inspection.

## Security (progressive)
Optional verified boot, image integrity checks, app isolation, permissions,
protected system files, signed system components. Must not block development.

## Future Physical Hardware
ARM64 or RISC-V 64-bit, modern RAM/storage/display, USB, NIC, keyboard,
optional Osborne-style enclosure. Same conceptual boot sequence with real
firmware/hardware detection instead of virtual detection.
