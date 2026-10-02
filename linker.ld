OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

SECTIONS
{
	__high_half__ = 0xc0000000;

	. = __high_half__;
	_kernel_start = .;

	.text ALIGN(4K): AT(ADDR(.text)- __high_half__) {
		*(.multiboot)
		*(.text)
   	}

	.rodata ALIGN(4K): AT(ADDR(.rodata)- __high_half__) {
		*(.rodata)
	}

	.data ALIGN(4K): AT(ADDR(.data)- __high_half__) {
		*(.data)
	}

	.bss  ALIGN(4K): AT(ADDR(.bss)- __high_half__) {
		*(.bss)
		. = ALIGN(4K);
		_boot_page_directory_start = . -__high_half__;
		. += 3072;
		_boot_page_directory_768 = . -__high_half__;
		. += 1024;
		_boot_page_table_start = . -__high_half__;
		. += 4096;
		. += 8192;
		_kernel_stack = .;
	}

	_kernel_end = .;
 }
