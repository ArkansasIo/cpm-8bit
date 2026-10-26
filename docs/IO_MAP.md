# I/O Map Specification

## Legacy Z80 Profile

The compatibility machine uses explicit Z80 I/O ports. The initial allocation is:

| Port | Function |
|---|---|
| 00h | Console status |
| 01h | Console data |
| 02h | Keyboard status |
| 03h | Keyboard data |
| 04h | Storage status |
| 05h | Storage command/data |
| 06h | Timer status |
| 07h | Timer control |
| 08h | Video status |
| 09h | Video command/data |
| 0Ah | Serial status |
| 0Bh | Serial data |
| 0Ch | Network status |
| 0Dh | Network data |
| 0Eh | System control |
| 0Fh | Diagnostic status |

These allocations are project-defined compatibility ports and are not intended to represent undocumented original Osborne hardware registers.

## Modern 64-bit Guest

Modern devices use Linux device drivers and platform interfaces. osbex should not depend on legacy Z80 port numbers outside the CP/M compatibility subsystem.

## Device Contract

Every virtual device should provide:

- Reset behavior.
- Read/write operations.
- Status reporting.
- Interrupt behavior where applicable.
- Error reporting.
- Deterministic test mode.

## Interrupts

Interrupt routing must be centralized in the virtual-machine interrupt controller. Devices must request interrupts rather than manipulating CPU state directly.

## Future Hardware

ARM64 and RISC-V board devices will use MMIO, platform buses, or native Linux drivers as appropriate. The osbex device abstraction should remain stable across these implementations.
