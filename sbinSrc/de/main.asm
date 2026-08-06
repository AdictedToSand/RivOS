global _start
BITS 32

_start:
    XOR eax, eax
    MOV edi, stdoutFp
    INT 0x80
    MOV dword [stdoutFd], eax

    MOV eax, 1
    MOV edi, dword [stdoutFd]
    MOV esi, msg
    MOV edx, msgLen
    INT 0x80

.hlt:
    JMP .hlt

section .data
msg: db "Hello, world!"
msgLen equ $ - msg
stdoutFp: db "/dev/stdlog", 0

section .bss
stdoutFd: resd 1
