BITS 32

global claim
global open
global write
global setFunc

open:
    MOV edi, dword [esp+4]
    XOR eax, eax
    INT 0x80
    RET

write:
    MOV edi, dword [esp+4]
    MOV esi, dword [esp+8]
    MOV edx, dword [esp+12]

    MOV eax, 1
    INT 0x80
    RET

claim:  
    MOV edi, dword [esp+4]
    MOV eax, 4
    INT 0x80
    RET

setFunc:
    MOV edi, dword [esp+4]
    MOV esi, dword [esp+8]
    MOV eax, 5
    INT 0x80
    RET

section .rodata

conts: db "Hello, world!", 10, 0
contsLen equ $ - conts
fp: db "/dev/stdout", 0
