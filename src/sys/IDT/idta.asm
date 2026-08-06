; Credits: https://wiki.osdev.org/Interrupts_Tutorial
BITS 32

extern exceptionHandler

struc InterruptFrame
    .edi:       resd 1
    .esi:       resd 1
    .ebp:       resd 1
    .esp:       resd 1
    .ebx:       resd 1
    .edx:       resd 1
    .ecx:       resd 1
    .eax:       resd 1

    .vector:    resd 1

    .errorCode: resd 1
    .eip:       resd 1
    .cs:        resd 1
    .eflags:    resd 1
    .userEsp:   resd 1
    .ss:        resd 1
endstruc


%macro isr_err_stub 1
isr_stub_%+%1:
    PUSH %1

    PUSHA 

    PUSH esp
    CALL exceptionHandler
    ADD esp, 4

    POPA

    ADD esp, 4 ; remove vector
    ADD esp, 4 ; remove CPU error code

    IRETD 
%endmacro


%macro isr_no_err_stub 1
isr_stub_%+%1:
    PUSH 0          ; fake error code
    PUSH %1         ; vector

    PUSHA

    PUSH esp
    CALL exceptionHandler
    ADD esp, 4

    POPA

    ADD esp, 8 ; vector + fake error code

    IRETD
%endmacro

isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
isr_no_err_stub 31

isr_no_err_stub 0x80

global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep

