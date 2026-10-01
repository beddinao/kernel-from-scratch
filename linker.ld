OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
{
	. = 0xc0100000;
	_kernel_start = .;

	.text ALIGN(4K): AT(ADDR(.text)-0xc0000000) {
		*(.multiboot)
		*(.text)
   	}

	.rodata ALIGN(4K): AT(ADDR(.rodata)-0xc0000000) {
		*(.rodata)
	}

	.data ALIGN(4K): AT(ADDR(.data)-0xc0000000) {
		*(.data)
	}

	.bss  ALIGN(4K): AT(ADDR(.bss)-0xc0000000) {
		*(.bss)
		. = ALIGN(4K);
		_boot_page_directory_start = . -0xc0000000;
		. += 3072;
		_boot_page_directory_768 = . -0xc0000000;
		. += 1024;
		_boot_page_table_start = . -0xc0000000;
		. += 4096;
		. += 8192;
		_kernel_stack = .;
	}

	_kernel_end = .;
 }
