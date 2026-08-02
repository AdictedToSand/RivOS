ENTRY(_start)

OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    . = 0x400000;

    .text ALIGN(4K) :
    {
        *(.text)
        *(.text.*)
    }

    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
        *(.rodata)
        *(.rodata.*)
    } /* TODO: This is evil as fuck */

    .bss ALIGN(4K) :
    {
        *(.bss)
        *(COMMON)
    }
}
