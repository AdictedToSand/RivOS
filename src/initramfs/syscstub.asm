section .bss
align 16
global liveSyscStackBottom
global liveSyscStackTop 
liveSyscStackBottom resb 16384
liveSyscStackTop:

section .text
global liveSyscStub 
extern liveSyscHandlr 

liveSyscStub:
    PUSHA
    MOV eax, esp
    MOV esp, liveSyscStackTop 
    PUSH eax ; save the interrupted esp so we can restore it after

    PUSH eax ; Push the esp of the process, not the current esp
    CALL liveSyscHandlr 
    ADD esp, 4

    POP eax ; restore the interrupted esp
    MOV esp, eax
    POPA
    IRETD
