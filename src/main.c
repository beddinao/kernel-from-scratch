#include <kernel.h>

void inspect_mb_info(uint32_t _magic, uint32_t mm_ptr) {
	volatile struct multiboot_info_st *mbd = (struct multiboot_info_st*)mm_ptr;

	//vga_putdword("\nmultiboot flags: 0x", mbd->flags);

	if (_magic != MB1_MAGIC_NM)
		panic("invalid multiboot protocol!");

	if ((mbd->flags >> 6) & 1) {
		/*vga_putdword("\nmb mmap addr: 0x", mbd->mmap_addr);
		vga_putdword("\nmb mmap length: 0x", mbd->mmap_length);
		vga_putchar('\n');*/

		uint8_t *ptr = (uint8_t*)mbd->mmap_addr;
		uint8_t *eptr = ptr + mbd->mmap_length;
		struct multiboot_mmap_st* mb_mmap;
		//uint64_t _tot_ram_size = 0;

		for (; ptr < eptr;) {

			mb_mmap = (struct multiboot_mmap_st*)ptr;

			//_tot_ram_size += ( mb_mmap->length_high << 32 | mb_mmap->length_low );
			
			/*vga_putdword("st: 0x", mb_mmap->base_addr_low);
			vga_putdword(" len: 0x", mb_mmap->length_low);
			vga_putdword(" t: 0x", mb_mmap->type);*/

			ptr += mb_mmap->size + 4;
			//vga_putchar('\n');
		}

		//vga_putdword("RAM size: 0x", (_tot_ram_size >> 32) & 0xffffffff);
		//vga_putdword("", _tot_ram_size & 0xffffffff);
		/*if (_tot_ram_size < MIN_RAM)
			panic("insufficant memory at least 10Mb");*/
	}
}

void print_hdr(uint32_t mm_ptr) {
	volatile struct multiboot_info_st *mbd = (struct multiboot_info_st*)mm_ptr;
	//           "                                                                               "
	  vga_putstr("          :::      ::::::::\n");
	  vga_putstr("        :+:      :+:    :+:\n");
	  vga_putstr("      +:+ +:+         +:+       ");
	if ((mbd->flags >> 9) & 1) {
		vga_putstr("BOSS: ");
		vga_putstr((uint8_t*)mbd->boot_loader_name);
	}
	vga_putchar('\n');
	vga_putdword("    +#+  +:+       +#+          kernel_start: 0x", (uint32_t)&_kernel_start); vga_putchar('\n');
	vga_putdword("  +#+#+#+#+#+   +#+             kernel_end  : 0x", (uint32_t)&_kernel_end); vga_putchar('\n');
	vga_putdword("       #+#    #+#               stack_top   : 0x", (uint32_t)&_kernel_stack); vga_putchar('\n');
	vga_putdword("      ###   ########            stack_bottom: 0x", (uint32_t)&_kernel_stack-KSTACK_S); vga_putchar('\n');
}

int kmain(uint32_t _magic, uint32_t mm_ptr) {

	vga_init();
	print_hdr(mm_ptr);

	inspect_mb_info(_magic, mm_ptr);

	pmm_init(mm_ptr);

	//print_init_info();

	gdt_init();
	idt_init();

	((struct vga_info_st*)VGA_INFO_ST)->attr |= (1 << 3);
	vga_putchar('\n');
	vga_update_cursor();

	while (1) 
	        asm volatile ("hlt");
}
