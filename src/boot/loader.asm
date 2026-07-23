; This file is the entry point to the PC 
; Since we only have 512 bytes to work with right now, the main job of this file is to load the "second stage"
; The second stage will contain all of the bootloader code
BITS 16
ORG 0x7C00

global _start

_start:
    MOV ah, 0x02
    MOV al, 1
    MOV ch, 0
    MOV cl, 2
    MOV dh, 0
    MOV dl, 0x80
    MOV bx, 0x8000

    INT 0x13
    JMP 0x0000:0x8000

; Disk failed to load
; At this point there is nothing we can do
; Just hang forever
diskError:
    CLI
.hlt:
    HLT
    JMP .hlt

times 510-($-$$) db 0
dw 0xAA55
