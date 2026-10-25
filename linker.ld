/* kernel/linker.ld — link kernel at 0x7E00 (right after bootsector) */
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
ENTRY(_kstart)

SECTIONS
{
    . = 0x7E00;

    .text : ALIGN(4) {
        *(.text._kstart)
        *(.text*)
    }

    .rodata : ALIGN(4) { *(.rodata*) }
    .data   : ALIGN(4) { *(.data*) }

    .bss : ALIGN(4) {
        __bss_start = .;
        *(.bss*) *(COMMON)
        __bss_end = .;
    }

    /DISCARD/ : { *(.eh_frame*) *(.comment*) *(.note*) *(.gcc_except_table*) }
}
