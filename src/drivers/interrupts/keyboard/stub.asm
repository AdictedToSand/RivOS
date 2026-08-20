section .bss
align 16
global keyboardStackBottom
global keyboardStackTop
keyboardStackBottom: resb 4096
keyboardStackTop:

section .text
global keyboardStub
extern keyboardHandler

keyboardStub:
    PUSHA
    MOV eax, esp
    MOV esp, keyboardStackTop
    PUSH eax
    CALL keyboardHandler
    POP eax
    MOV esp, eax
    POPA
    IRETD
