global irq1Stub
extern irq1Handler

irq1Stub:
    pusha

    call irq1Handler

    popa
    iretd
