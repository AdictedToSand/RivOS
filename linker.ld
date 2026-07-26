/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)

PHDRS
{
	kernel PT_LOAD FILEHDR PHDRS FLAGS(7);
}

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* It used to be universally recommended to use 1M as a start offset,
	   as it was effectively guaranteed to be available under BIOS systems.
	   However, UEFI has made things more complicated, and experimental data
	   strongly suggests that 2M is a safer place to load. In 2016, a new
	   feature was introduced to the multiboot2 spec to inform bootloaders
	   that a kernel can be loaded anywhere within a range of addresses and
	   will be able to relocate itself to run from such a loader-selected
	   address, in order to give the loader freedom in selecting a span of
	   memory which is verified to be available by the firmware, in order to
	   work around this issue. This does not use that feature, so 2M was
	   chosen as a safer option than the traditional 1M. */
	. = 0x100000;

    kernelStart = .;

    .header BLOCK(4K) : ALIGN(4K)
    {
        KEEP(*(.header))
    } :kernel

	/* First put the multiboot header, as it is required to be put very early
	   in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text ALIGN(4K) :
	{
		*(.multiboot)
		*(.text)
		*(.text.*)
	} :kernel

	/* Read-only data. */
	.rodata ALIGN(4K) :
	{
		*(.rodata)
		*(.rodata.*)
	} :kernel

	/* Read-write data (initialized) */
	.data ALIGN(4K) :
	{
		*(.data)
		*(.data.*)
		*(.ctors)
		*(.dtors)
	} :kernel

	/* Read-write data (uninitialized) and stack */
	bssStart = .;

	.bss ALIGN(4K) :
	{
		*(COMMON)
		*(.bss)
		*(.bss.*)
	} :kernel

	bssEnd = .;

	heapStart = .;
	. += 64M;
	heapEnd = .;

	kernelEnd = .;

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */
}

bssSize = bssEnd - bssStart;
heapSize = heapEnd - heapStart;
kernelSize = kernelEnd - kernelStart - bssSize - heapSize;
