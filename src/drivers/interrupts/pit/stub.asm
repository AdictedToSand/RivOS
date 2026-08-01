global pitStub
extern pitHandler

pitStub:
    PUSHA

    CALL pitHandler

    POPA
    IRETD
