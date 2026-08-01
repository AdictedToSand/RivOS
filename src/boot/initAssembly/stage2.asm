BITS 16
global _start

JMP _start

gdtStart:

gdtNull:
    dq 0x0000000000000000

gdtCode:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

gdtData:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

gdtEnd:

gdtDescriptor:
    dw gdtEnd - gdtStart - 1
    dd gdtStart

biosPuts:
   pusha

.loop:
    lodsb ; AL = [DS:SI], SI++
    test al, al ; Check for null terminator
    jz .done

    mov ah, 0x0E ; BIOS teletype output
    mov bh, 0x00 ; Page number
    int 0x10

    jmp .loop

.done:
    popa
    RET    

_start:
    MOV [bootDrive], dl ; dl will contain the bootDrive (passed on by init)

    ; Right now the only thing we need to is get to protected mode.
    

%define a20Failed a20NotSupported
; Hack, should be fixed!

; Credits: wiki.osdev.org/A20_Line
.enableA20Line:
    CLI ; It is required to stop interrupts for entering protected mode
    MOV ax, 0x2403 ; Query A20 gate support
    INT 0x15
    JC a20NotSupported

    TEST ah, ah
    JNZ a20NotSupported

    MOV ax, 0x2402 ; Get A20 gate status
    INT 0x15
    JC a20Failed ; Could not get status
    TEST ah, ah
    JNZ a20Failed ; Could not get status
    TEST al, al
    JNZ .a20Activated ; A20 is already activated!

    MOV ax, 0x2401
    INT 0x15
    JC a20Failed
    TEST ah, ah
    JNZ a20Failed

.a20Activated:
    LGDT [gdtDescriptor]
    ; We can't directly do bitwise ops (OR) on cr0 so we load it into eax and then store it back
    MOV eax, cr0
    OR eax, 1
    MOV cr0, eax

    ; Now we do a far jump to protected mode
    JMP 08h:pmodeInit

endOfProgram:

    CLI
.hlt:
    HLT
    JMP .hlt

a20NotSupported:
    MOV si, a20ErrMsg
    CALL biosPuts
    
    CLI
.hlt:
    HLT
    JMP .hlt


bootDrive: db 0

a20ErrMsg: db "There was a problem when loading the bootloader. RivBoot will not continue. A reboot is recommended", 0

BITS 32
extern startBoot

stackBottom:
    times 16384 db 0
stackTop:

pmodeInit:
    MOV esp, stackBottom
    JMP startBoot
    ; Hopefully we don't go past this.
