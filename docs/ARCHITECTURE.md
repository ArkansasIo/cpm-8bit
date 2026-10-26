# osbex 2.2 System Architecture

## Purpose

Define the major layers and contracts of the Osborne-inspired osbex 2.2 platform.

## Runtime Layers

```text
Windows Host
  -> osbex Virtual Machine / Emulator
     -> Firmware / POST / Boot Manager
        -> 64-bit Linux Kernel
           -> Initramfs
              -> osbex 2.2 Services
                 -> osbex C++23 API
                    -> GUI / Applications
```

The CP/M compatibility subsystem is a separate guest environment:

```text
osbex 2.2
  -> CP/M Compatibility Manager
     -> Z80 CPU Emulation
        -> CP/M 2.2 BIOS/BDOS-compatible environment
           -> Osborne disk images
```

## Separation of Responsibilities

### Host

Windows owns the native window, audio backend, filesystem sandbox, networking bridge, process lifetime, and installation/update integration.

### Virtual Machine

The VM owns virtual CPU state, guest RAM, storage devices, display, keyboard, timers, interrupts, and device configuration.

### Linux

Linux owns scheduling, memory management, processes, networking, security primitives, drivers, and system calls.

### osbex

osbex owns the user-facing operating environment, services, compatibility manager, application model, configuration, desktop, and Osborne-inspired experience.

### Applications

Applications use the versioned osbex API rather than depending directly on emulator internals.

## Interface Rules

1. Emulator internals must not be exposed directly to applications.
2. osbex services communicate through explicit interfaces and IPC where appropriate.
3. CP/M software is sandboxed from host resources.
4. Device access is mediated by virtual devices and Linux drivers.
5. Public C++23 APIs require versioning and compatibility rules.
6. Hardware-specific code must be isolated behind architecture interfaces.

## Runtime Modes

### Modern Mode

Boot a 64-bit Linux guest and osbex 2.2.

### CP/M Mode

Boot a Z80/CP/M-compatible guest environment from an Osborne disk image.

### Recovery Mode

Start only the minimum diagnostics, filesystem, configuration, and repair services required to recover the installation.

### Diagnostics Mode

Run hardware and subsystem tests without starting the full desktop.

## Future Hardware

The same logical interfaces should be portable to ARM64 and RISC-V boards. Host-specific services are replaced by firmware, board support, and native Linux drivers.
