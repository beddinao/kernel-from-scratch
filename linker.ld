OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
{
	. = 1M;
	_kernel_start = .;
	.text : 
	{
		*(.multiboot)
		*(.text)
   	}
	.data : { *(.data) }
	.bss  : {
		*(.bss)
		. += 8192;
		_kernel_stack = .;
	}
	_kernel_end = .;
 }
