; src/interrupts.asm — kernel-side assembly
; real-mode interrupt stubs, ISR wrappers and kernel entry.
BITS 16
SECTION .text

GLOBAL _kstart
EXTERN _kmain
EXTERN kmain
EXTERN __bss_start_seg
EXTERN __bss_start_off
EXTERN __bss_words

; underscore-decoration bridge: clang/LLVM emits undecorated ELF symbols,
; so map the asm's underscored externs onto the plain C++ names.
%define _kmain kmain
%define _hw_irq_handler hw_irq_handler
%define _dos_int21_handler dos_int21_handler

; C++23 freestanding helpers
EXTERN __cxa_pure_virtual

; -----------------------------------------------
; _kstart: kernel entry point (called by bootsector at 0x7E00)
; -----------------------------------------------
_kstart:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ax, 0xB800
    mov fs, ax                  ; FS = VGA segment (kept for kernel debug)
    xor ax, ax
    mov ss, ax
    mov sp, 0x7000              ; stack below kernel
    cld
    ; zero .bss — C++ statics (singletons, vtables, RamFs) need this.
    ; Use a full 32-bit pointer: .bss can start above 64K in this layout.
    mov ax, __bss_start_seg
    mov es, ax                  ; ES:DI = __bss_start (16:16 far form)
    mov di, __bss_start_off
    mov cx, __bss_words
    mov ax, 0
    jcxz .bss_done
    rep stosw
.bss_done:
    mov ax, 0xB800
    mov fs, ax                  ; FS = VGA segment (DS/ES kept for kernel)
    mov word [fs:0], 0x0F41     ; 'A' marker: proves _kstart started
    sti
    call _kmain                  ; enter C++ world
.halt:
    hlt
    jmp .halt

; -----------------------------------------------
; wrapper: call C++ interrupt handler with a stub frame
; -----------------------------------------------
GLOBAL _irq_stub
GLOBAL irq_stub
EXTERN _hw_irq_handler
irq_stub:
_irq_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    call _hw_irq_handler
    pop gs
    pop fs
    pop es
    pop ds
    popa
    iret

; -----------------------------------------------
; int 21h gate: DOS API call router (called by user programs)
; -----------------------------------------------
GLOBAL _dos_api_gate
GLOBAL dos_api_gate
EXTERN _dos_int21_handler
dos_api_gate:
_dos_api_gate:
    pusha
    push ds
    push es
    call _dos_int21_handler      ; AH = function, DL/DH = 8-bit args
    pop es
    pop ds
    popa
    iret

; -----------------------------------------------
; C++ runtime stubs (freestanding)
; -----------------------------------------------
GLOBAL __cxa_pure_virtual_impl
__cxa_pure_virtual_impl:
    hlt
    jmp __cxa_pure_virtual_impl
