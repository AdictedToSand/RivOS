ENTRY(_start)

SECTIONS {
    . = 0x8000;

    bootloaderStart = .;

    .text : { *(.text .text.*) }
    .rodata : { *(.rodata .rodata.*) }
    .data : { *(.data .data.*) }

    .bss (NOLOAD) : {
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(16);
        stackBottom = .;
        . += 0x4000;
        stackTop = .;
    }

    /DISCARD/ : {
        *(.comment)
        *(.note*)
        *(.eh_frame)
    }

    bootloaderEnd = .;
}
