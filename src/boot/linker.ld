ENTRY(_start)

PHDRS {
    text PT_LOAD FLAGS(5); /* R + X */
    data PT_LOAD FLAGS(6); /* R + W */
}

SECTIONS {
    . = 0x8000;

    bootloaderStart = .;

    .text : {
        *(.text .text.*)
    } :text

    .rodata : {
        *(.rodata .rodata.*)
    } :text

    .data : {
        *(.data .data.*)
    } :data

    .bss (NOLOAD) : {
        *(.bss .bss.*)
        *(COMMON)

        . = ALIGN(16);
        stackBottom = .;
        . += 0x4000;
        stackTop = .;
    } :data

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }

    bootloaderEnd = .;
}
