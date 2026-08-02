BITS 32
global syscallStub
extern syscallHandler

syscallStub:
    PUSHA

    PUSH esp
    CALL syscallHandler
    ADD esp, 4

    POPA
    IRETD
