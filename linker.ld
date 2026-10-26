/* kernel/linker.ld � link kernel at 0x7E00 (right after bootsector) */
OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
ENTRY(_kstart)
SECTIONS
{
    . = 0x7E00;
    .text : ALIGN(4) {
        KEEP(*(.text._kstart))
        *interrupts*(.text)
        *(.text*)
    }
    .rodata : ALIGN(4) { *(.rodata*) }
    .data   : ALIGN(4) { *(.data*) }
    .bss : ALIGN(16) {
        __bss_start = .;
        *(.bss*) *(COMMON)
        __bss_end = .;
    }
    /* real-mode segment:offset decomposition of the .bss start.
       ALIGN(16) guarantees off <= 0xF, so one segment covers everything
       up to 64 KiB of bss. */
    __bss_start_off  = __bss_start & 0xF;
    __bss_start_seg  = (__bss_start >> 4) & 0xFFFF;
    __bss_words      = (__bss_end - __bss_start) >> 1;
    /DISCARD/ : { *(.eh_frame*) *(.comment*) *(.note*) *(.gcc_except_table*) }
}
