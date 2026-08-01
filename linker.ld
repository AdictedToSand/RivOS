PHDRS
{
    text PT_LOAD FILEHDR PHDRS FLAGS(5); /* R-X */
    data PT_LOAD               FLAGS(6); /* RW- */
}
SECTIONS
{
    . = 0x100000;
    kernelStart = .;
    .header BLOCK(4K) : ALIGN(4K)
    {
        KEEP(*(.header))
    } :text
    .text ALIGN(4K) :
    {
        *(.text)
        *(.text.*)
    } :text
    .rodata ALIGN(4K) :
    {
        *(.rodata)
        *(.rodata.*)
    } :text

    .data ALIGN(4K) :
    {
        *(.data)
        *(.data.*)
    } :data

    .ctors ALIGN(4) : {
        ctorsStart = .;
        KEEP(*(SORT(.ctors)))
        KEEP(*(SORT(.ctors.*)))
        ctorsEnd = .;
    } :data

    bssStart = .;
    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    } :data
    bssEnd = .;
    heapStart = .;
    . += 64M;
    heapEnd = .;
    kernelEnd = .;
}
bssSize = bssEnd - bssStart;
heapSize = heapEnd - heapStart;
kernelSize = kernelEnd - kernelStart - bssSize - heapSize;



