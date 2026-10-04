#include <kernel.h>

int kmain(uint32_t _magic, uint32_t mm_ptr) {

	struct vmm_pde boot_pd = {
		(struct vmm_pde*)(
			(uint32_t)&_boot_page_directory_start
			+ (uint32_t)&__high_half__
	)};

	vmm_set_pde(&boot_pd, 0, 0);

	vga_init();

	print_hdr(mm_ptr);
	pmm_init(mm_ptr, inspect_mb_info(_magic, mm_ptr));

	gdt_init();
	idt_init();

	vmm_init();

	((struct vga_info_st*)&_vga_info_st)->attr |= (1 << 3);
	vga_putchar('\n');
	vga_update_cursor();

	//*((uint8_t*)0) = 0;

	while (1) 
	        asm volatile ("hlt");

}
