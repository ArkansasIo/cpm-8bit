; programs/hello.asm — MS-DOS 2.2 style user program (COM-style)
; Uses INT 21h AH=09h (print $-string) and AH=4Ch (terminate), all 8-bit args.
ORG 0x100
BITS 16

start:
    mov ah, 09h                  ; DOS 2.2: write string
    mov dx, msg
    int 21h
    mov ah, 4Ch                  ; DOS 2.2: terminate
    mov al, 0                    ; 8-bit exit code
    int 21h

msg db "Hello from MS-DOS 2.2 API on OS Prototype 3.0.0!", 13, 10, '$'
