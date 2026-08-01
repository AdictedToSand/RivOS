global keyboardStub
extern keyboardHandler

keyboardStub:
    PUSHA

    CALL keyboardHandler

    POPA
    IRETD
