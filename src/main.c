#include <kernel.h>

int kmain(uint32_t _magic, uint32_t mm_ptr) {

	vmm_set_pde(0, 0);

	vga_init();
	print_hdr(mm_ptr);
	
	pmm_init(mm_ptr, inspect_mb_info(_magic, mm_ptr));

	gdt_init();
	idt_init();

	vmm_init();
	
	heap_init();

	kshell_enable();

	while (1) 
	        asm volatile ("hlt");
}
