BITS 16
ORG 0x8000

; Otherwise whatever function starting first would just execute
; This makes sure _start is always the entry point
JMP _start

; Outputs a string to the screen
;   Parameters:
;       - Si contains a pointer to the string
;       - Dx contains the length of the string
;   Clobbered:
;       - Ah
;       - Al
;       - Si
;       - Dx
writes:
.loop:
    CMP dx, 0
    JE .done

    MOV ah, 0x0E
    MOV al, [si]
    INT 0x10

    INC si
    DEC dx

    JMP .loop

.done:
    RET
; Returns the length of a null terminated string in ax
;   Parameters:
;       - Si contains the string to determine the length
;   Clobbered:
;       - Si will contain the address of the null terminator
strlen:
    XOR ax, ax
.loop:
    CMP byte [si], 0
    JE .done

    INC ax
    INC si
    JMP .loop

.done:
    RET

; Outputs a string to the screen
;   Parameters: 
;       - Si will contain the null terminated string
;   Clobbered:
;       - Ax will contain the length of the string
;       - Dx will be 0
puts:
    PUSH si ; Si will be clobbered afterwards
    ; Si already contains the string
    CALL strlen

    POP si ; Restore the string in si
    MOV dx, ax
    CALL writes

    RET

clearscreen:
    MOV ax, 0x00
    MOV al, 0x03
    INT 0x10

    RET

; Checks if two strings are equal
;   Parameters:
;       - Si contains a pointer to the first string
;       - Di contains a pointer to the second string
;   Returns:
;       - Ax contains 1 if the strings are equal
;       - Ax contains 0 if the strings are different
;   Clobbered:
;       - Al
streq:
.loop:
    MOV al, [si]
    CMP al, [di]
    JNE .notEqual

    CMP al, 0
    JE .equal

    INC si
    INC di
    JMP .loop

.equal:
    MOV ax, 1
    RET

.notEqual:
    MOV ax, 0
    RET

; Tries to find a kernel that is considered bootable. 
;   Params:
;       None
;   Returns:
;       Nothing, however in the kernelInfo variable will now be all the correct data
;   Clobbered:
;       Assume each register is clobbered
;       
struc KernelInfo
    .drive: resb 1
    .sector: resw 1
    .sizeAddr: resw 1 ;
    .entryPointAddr: resw 1 ; Where to look for these variabless
endstruc

kernelInfo:
    istruc KernelInfo
        at KernelInfo.drive, db 0
        at KernelInfo.sector, dw 0
        at KernelInfo.sizeAddr, dw 0
        at KernelInfo.entryPointAddr, dw 0
    iend

findKernel:
    MOV cl, 0x02

.loop:
    XOR ax, ax
    MOV es, ax

    MOV ah, 0x02
    MOV al, 0x01 ; read one sector
    MOV ch, 0 ; Cylinder
    MOV bx, 0x9000 ; Where to put the sector
    MOV dh, 0 ; head
    MOV dl, 0x80 ; Hard disk
    PUSH cx
    INT 0x13
    POP cx

    MOV si, bootableString
    MOV di, 0x9000
    CALL streq

    CMP ax, 1
    JE .kernelFound

    INC cl

.kernelFound:
    ; TODO: Reduce comments
    MOV [kernelInfo + KernelInfo.drive], 0x80
    MOV [kernelInfo + KernelInfo.sector], cx
    
    MOV dx, 0x9000 + bootableStringLen
    MOV [kernelInfo + KernelInfo.entryPointAddr], dx

    MOV dx, 0x9000 + bootableStringLen + 4 ; sizeof(long)
    MOV [kernelInfo + KernelInfo.sizeAddr], dx

    RET

.diskError:
    MOV si, readErrorMsg
    CALL puts

    CLI
.hlt:
    HLT
    JMP .hlt

; GDT code
gdtStart:

gdtNull:
    dq 0

gdtCode:
    dw 0xFFFF ; Limit low
    dw 0x0000 ; Base low
    db 0x00 ; Base middle
    db 0b10011010  ; Access
    db 0b11001111 ; Flags + limit high
    db 0x00 ; Base high

gdtData:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0b10010010
    db 0b11001111
    db 0x00

gdtEnd:

gdtDescriptor:
    dw gdtEnd - gdtStart - 1
    dd gdtStart

CODE_SEG equ gdtCode - gdtStart
DATA_SEG equ gdtData - gdtStart

enterProtectedMode:
    CLI

    LGDT [gdtDescriptor]

    MOV eax, cr0
    OR eax, 1
    MOV cr0, eax

    jmp CODE_SEG:initProtectedMode

_start:
    MOV [bootDrive], dl

    XOR ax, ax
    MOV ds, ax

    CALL clearscreen
    CALL findKernel

    MOV si, kernelFoundMsg
    MOV bl, 0x04
    CALL puts

    JMP enterProtectedMode

.hlt:
    CLI
    HLT
    JMP .hlt

bootDrive: db 0
kernelFoundMsg: db "Kernel found", 0
bootableString: db "BOOTABLE", 0
bootableStringLen equ $ - bootableString
readErrorMsg: db "Disk read failed", 0

bootloaderEnd equ $

BITS 32

; Right now we have no BIOS interrupts so previous functions are useless
; Luckily the HDD sector contents are still loaded at the addresses told
initProtectedMode:
    MOV ax, DATA_SEG
    MOV ds, ax
    MOV es, ax
    MOV fs, ax
    MOV gs, ax
    MOV ss, ax

    MOV byte [0xB8000], 'A'
    MOV byte [0xb8001], 0x0F

    CLI
.hlt:
    hlt
    JMP .hlt

%if bootloaderEnd - $$ > 512
    %error "Bootloader is too large!"
%endif
