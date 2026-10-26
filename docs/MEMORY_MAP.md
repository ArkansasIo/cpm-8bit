# Memory Map Specification

## Scope

This document defines the initial memory model for the Osborne/CP/M compatibility machine and the modern 64-bit guest.

## Legacy Z80 / CP/M Profile

The Z80 compatibility profile exposes a 64 KiB logical address space.

| Range | Purpose |
|---|---|
| 0000h-00FFh | Reset/vector and low-memory area |
| 0100h-BFFFh | CP/M transient/program area |
| C000h-CFFFh | Reserved for compatibility services |
| D000h-D7FFh | Video/console compatibility window |
| D800h-DFFFh | Device/IO shadow area |
| E000h-EFFFh | ROM/firmware workspace or reserved |
| F000h-FFFFh | Firmware/monitor/stack region |

This is an initial design allocation, not a claim about original Osborne hardware. The final CP/M profile must be validated against the selected BIOS and disk-image compatibility target.

## Modern 64-bit Guest

Modern osbex uses the memory map supplied by the selected 64-bit Linux guest and boot firmware. osbex must query memory through the platform abstraction rather than hard-coding a capacity.

The boot display therefore reports detected/configured guest RAM, for example:

```text
MEMORY ............ 16384 MB OK
```

The value must originate from the VM configuration or physical firmware memory map.

## Physical Hardware Requirement

On dedicated hardware, the bootloader must consume the firmware-provided physical memory map and reserve regions for firmware, kernel, initramfs, device mappings, framebuffer, and runtime services as applicable.

## Memory Safety Rules

- No subsystem may assume a fixed amount of RAM.
- Reserved regions must never be allocated as general RAM.
- CP/M guest memory is isolated from host memory.
- Memory tests must identify the tested address range.
- Allocation failures must be reported through diagnostics.

## Future Work

- Formal ARM64 memory map.
- Formal RISC-V memory map.
- DMA and MMIO reservation rules.
- Kernel/initramfs load addresses.
- Guard pages and debugging memory protection.
