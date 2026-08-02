ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5); /* R + X */
    data PT_LOAD FLAGS(6); /* R + W */
}

SECTIONS
{
    . = 0x8000;
    bootStart = .;

    .text :
    {
        *(.text)
    } :text

    .rodata :
    {
        *(.rodata)
    } :text

    .data :
    {
        *(.data)
    } :data

    .bss :
    {
        *(COMMON)
        *(.bss)
    } :data

    bootEnd = .;
}
