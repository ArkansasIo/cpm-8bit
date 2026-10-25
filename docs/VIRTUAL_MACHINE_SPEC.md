# Virtual Machine Specification

## Goal

Provide a deterministic Osborne-inspired virtual computer that runs as a Windows application and can later be replaced by physical hardware.

## Machine Profiles

### Osborne-1 Compatibility

- Z80-compatible CPU.
- 64 KiB logical address space.
- CP/M-compatible boot environment.
- Osborne disk-image support.
- Character-oriented display model.
- Keyboard and serial-style device model.

### Modern-64

- 64-bit guest CPU architecture.
- Configurable guest RAM.
- Virtual block storage.
- Framebuffer/display device.
- Keyboard and pointer input.
- Virtual network adapter.
- Optional audio device.
- Timers and interrupt controller.

## VM Components

```text
VM Controller
├── CPU
├── Memory
├── Firmware
├── Boot Manager
├── Storage Controller
├── Display Controller
├── Keyboard Controller
├── Timer
├── Interrupt Controller
├── Network Adapter
└── Audio Adapter
```

## Configuration

A machine profile should define CPU model, RAM, storage, display, input, network, audio, boot order, and compatibility mode.

Example conceptual configuration:

```toml
[machine]
profile = "modern-64"
memory_mb = 16384

[storage]
boot_image = "images/mstos.img"

[display]
width = 1280
height = 800

[network]
enabled = true

[boot]
mode = "mstos"
```

## Determinism

The emulator should support a deterministic test mode for CPU, memory, device, boot, and CP/M regression tests.

## Snapshots

Snapshots should capture CPU state, RAM, virtual devices, storage metadata, timers, and configuration state. Snapshot formats must be versioned.

## Debugger

The VM debugger should eventually expose registers, memory, disassembly, breakpoints, watchpoints, instruction tracing, device state, and boot logs.

## Host Isolation

Guest software must not receive unrestricted Windows filesystem, process, registry, device, or network access. Host integrations are explicitly granted through virtual devices and sandboxed services.

## Performance

Performance targets will be measured rather than assumed. Benchmarks should include boot time, CP/M instruction throughput, Linux guest startup, GUI frame rate, storage throughput, and host CPU/RAM utilization.
