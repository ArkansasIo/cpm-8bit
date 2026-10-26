; boot/boot.asm — 512-byte bootsector
; Loads kernel.bin from floppy sectors 2.. to 0x7E00 and jumps to it.
; Redesigned loader: reads up to 60 sectors (~30 KiB) across tracks/heads,
; so the kernel can grow well past the old 18-sector (9 KiB) limit.
BITS 16
ORG 0x7C00

SECTORS_PER_TRACK equ 18
HEADS             equ 2
KERNEL_SECTORS    equ 60                  ; ~30 KiB kernel capacity

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

    ; multi-track read: kernel loads at 0x7E00 (ES:BX)
    mov ax, 0x07E0
    mov es, ax
    xor bx, bx                   ; ES:BX = 0x7E00
    mov word [Remaining], KERNEL_SECTORS
    mov cx, 0x0002               ; CH=cylinder 0, CL=sector 2
    mov dh, 0                    ; head 0
    mov dl, [BootDrive]

.ReadLoop:
    ; count = min(Remaining, sectors left on this track = 19 - CL)
    mov al, 19
    sub al, cl
    mov ah, 0
    cmp ax, [Remaining]
    jbe .CountOk
    mov ax, [Remaining]
.CountOk:
    test ax, ax
    jz .Done
    mov [ThisCount], al
    mov ah, 0x02                 ; BIOS read sectors into ES:BX
    int 0x13
    jc DiskError
    ; advance destination pointer: BX += AL*512, wrap into ES
    mov [SectorsRead], al
    mov ah, 0
    shl ax, 1
    shl ax, 1
    shl ax, 1
    shl ax, 1
    shl ax, 1
    shl ax, 1
    shl ax, 1
    shl ax, 1
    shl ax, 1
    add bx, ax
    jnc .NoWrap
    mov dx, es
    add dx, 0x1000
    mov es, dx
    and bx, 0xFFFF
.NoWrap:
    ; advance CHS past the sectors read
    mov al, [SectorsRead]
    add cl, al
    sub [Remaining], al
    cmp cl, SECTORS_PER_TRACK+1
    jb .NextCheck
    mov cl, 1
    inc dh
    cmp dh, HEADS
    jb .NextCheck
    mov dh, 0
    inc ch                       ; next cylinder
.NextCheck:
    cmp word [Remaining], 0
    ja .ReadLoop
.Done:
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
BootDrive   db 0
SectorsRead db 0
ThisCount   db 0
Remaining   dw 0

times 510-($-$$) db 0
dw 0xAA55                        ; boot signature
