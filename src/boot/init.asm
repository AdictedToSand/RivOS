BITS 16
ORG 0x7C00

JMP _start

puts:
    PUSHA
.loop:
    LODSB ; AL = [DS:SI], SI++
    TEST al, al ; Check for null terminator
    JZ .done

    MOV ah, 0x0E ; BIOS teletype output
    MOV bh, 0x00 ; Page number
    INT 0x10

    JMP .loop

.done:
    POPA
    RET

_start:
    XOR ax, ax
    MOV ds, ax

    MOV [bootDrive], dl ; Dl is guaranteed to contain the bootDrive at start of execution

    XOR ax, ax
    MOV es, ax ; Segment

    MOV ch, 0 ; Cylinder
    MOV cl, 2 ; Sector: 2 skips past the inital boot sector. (And this is one indexed)
    MOV dh, 0 ; Head
    MOV dl, [bootDrive] ; Load from the boot drive
    MOV bx, 0x8000 ; Where to load sectors
    MOV al, STAGE2_SECTORS

    MOV ah, 0x02 ; BIOS read sectors
    INT 0x13

    JC driveErr

    MOV dl, [bootDrive]
    jmp 0x0000:0x8000

driveErr:
    MOV si, diskErrMsg
    CALL puts

    CLI
.hlt:
    hlt
    JMP .hlt

bootDrive: resb 1

diskErrMsg: db "A disk error occurred and execution can not continue.", 0

times 510-($-$$) db 0
dw 0xAA55