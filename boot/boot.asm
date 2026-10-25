; boot/boot.asm — 512-byte bootsector
; Loads kernel.bin from floppy sectors 2.. to 0x7E00 and jumps to it.
BITS 16
ORG 0x7C00

Start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [BootDrive], dl          ; BIOS passes boot drive in DL

    ; print banner
    mov si, MsgBoot
    call Print

    ; read sectors: kernel loads at 0x7E00 (ES:BX)
    mov ax, 0x07E0
    mov es, ax
    xor bx, bx                   ; ES:BX = 0x7E00
    mov cx, 0x0002               ; CH=cylinder 0, CL=sector 2
    mov dh, 0                    ; head 0
    mov dl, [BootDrive]
    mov ah, 0x02                 ; BIOS read sectors
    mov al, 18                   ; read 18 sectors (9 KiB kernel max)
    int 0x13
    jc DiskError

    ; success: jump to kernel
    mov si, MsgOk
    call Print
    jmp 0x0000:0x7E00

DiskError:
    mov si, MsgErr
    call Print
.halt:
    hlt
    jmp .halt

; Print ASCIZ string at DS:SI
Print:
    pusha
.next:
    lodsb
    or al, al
    jz .done
    mov ah, 0x0E                 ; BIOS teletype (8-bit AH/AL)
    mov bh, 0
    int 0x10
    jmp .next
.done:
    popa
    ret

MsgBoot db "TinyOS-8 booting...", 13, 10, 0
MsgOk   db "kernel loaded.", 13, 10, 0
MsgErr  db "DISK ERROR!", 13, 10, 0
BootDrive db 0

times 510-($-$$) db 0
dw 0xAA55                        ; boot signature
