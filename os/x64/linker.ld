/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(_start)


/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	. = 2M;

	kernel64_start = .;

	/* First put the multiboot header, as it is required to be put very early
	   in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
    .entry BLOCK(4K) : ALIGN(4K)
    {
        *(.entry)
    }
    
	.text BLOCK(4K) : ALIGN(4K)
	{
		*(.text)
	}

	/* Read-only data. */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) and stack */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}

	/* The compiler may produce other sections, by default it will put them in
	   a segment with the same name. Simply add stuff here as needed. */

    . += 0x8000;
    stack_top = ALIGN(4K);

    . += 0x1000;

    kernel64_end = ALIGN(4K);
}