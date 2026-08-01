global irq0Stub
extern irq0Handler

irq0Stub:
    pusha

    call irq0Handler

    popa
    iretd
