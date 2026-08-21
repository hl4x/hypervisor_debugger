/* The bootloader will look at this image and start execution at the symbol
   designated as the entry point. */
ENTRY(start32)

SECTIONS
{
	/* Load the kernel starting at 1 MiB */
	. = 1M;

	/* 32-bit boot */
	.boot BLOCK(4K) : ALIGN(4K)
	{
		KEEP(*(.multiboot))
		*(.boot.text)
	}

	/* 64-bit kernel */
	.text BLOCK(4K) : ALIGN(4K)	
	{
		*(.text)
	}

	/* Read-only data (e.g., string literals) */
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	/* Read-write data (initialized) */
	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	/* Read-write data (uninitialized) */
	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}