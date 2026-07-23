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
;       - Ah will contain 0xE
;       - Al will contain the last character
;       - Si will contain the last char
;       - Dx will contain 0
writes:
.loop:
    CMP dx, 0
    JE .done

    MOV ah, 0xE
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

findKernel:
    

    RET

_start: 
    CALL clearscreen

    XOR al, al

    MOV si, string
    CALL puts

    CLI
.hlt:
    HLT
    JMP .hlt

string: db "hello, world", 0
