#include <kernel.h>

int kmain(uint32_t _magic, uint32_t mm_ptr) {

	*((uint32_t*)((uint32_t)&_boot_page_directory_start
			+ (uint32_t)&__high_half__)) = 0;

	asm volatile (
		"mov eax, cr3\n"
		"mov cr3, eax"
	);

	vga_init();
	print_hdr(mm_ptr);
	
	pmm_init(mm_ptr, inspect_mb_info(_magic, mm_ptr));

	gdt_init();
	idt_init();

	vmm_init();

	kshell_enable();

	//*((uint8_t*)0) = 0;

	while (1) 
	        asm volatile ("hlt");
}
