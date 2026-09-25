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
	struct vga_info_st* vga_info = (struct vga_info_st*)VGA_INFO_ST;
	uint8_t old_color = vga_info->_sc[ vga_info->attr & 0b11 ]._color;
	//           "                                                                               "
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 3;
	vga_putstr("          :::      ::::::::\n");
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 3;
	vga_putstr("        :+:      :+:    :+:\n");
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 11;
	vga_putstr("      +:+ +:+         +:+       ");
	if ((mbd->flags >> 9) & 1) {
		vga_putstr("BOSS: ");
		vga_putstr((uint8_t*)mbd->boot_loader_name);
	}
	vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 11;
	vga_putdword("    +#+  +:+       +#+          kernel_start: 0x", (uint32_t)&_kernel_start); vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 15;
	vga_putdword("  +#+#+#+#+#+   +#+             kernel_end  : 0x", (uint32_t)&_kernel_end); vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 15;
	vga_putdword("       #+#    #+#               stack_top   : 0x", (uint32_t)&_kernel_stack); vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 11;
	vga_putdword("      ###   ########            stack_bottom: 0x", (uint32_t)&_kernel_stack-KSTACK_S); vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color;
}

int kmain(uint32_t _magic, uint32_t mm_ptr) {
	/*volatile struct multiboot_info_st *mbd = (struct multiboot_info_st*)mm_ptr;
	if (_magic != 0x2BADB002)
		return 0;

	if (((mbd->flags >> 12) & 1) && mbd->framebuffer_type == 1) {
		uint8_t *vram = (uint8_t*)mbd->framebuffer_addr;
		for (uint32_t y = 0; y < mbd->framebuffer_height; y++) {
			for (uint32_t x = 0; x < mbd->framebuffer_width; x++) {
				uint8_t *pixel = vram + x*(mbd->framebuffer_bpp/8);

				uint32_t color = (0xffU << mbd->framebuffer_red_field_position)|
					(0xffU << mbd->framebuffer_green_field_position)|
					(0xffU << mbd->framebuffer_blue_field_position);

				if (mbd->framebuffer_bpp == 32)
					*((uint32_t*)pixel) = color;
				else {
					pixel[0] = color;
					pixel[1] = color >> 8;
					pixel[2] = color >> 16;
				}
			}
			vram += mbd->framebuffer_pitch;
		}
	}

	return 0;
	*/
	vga_init();
	print_hdr(mm_ptr);

	inspect_mb_info(_magic, mm_ptr);

	pmm_init(mm_ptr);

	gdt_init();
	idt_init();

	((struct vga_info_st*)VGA_INFO_ST)->attr |= (1 << 3);
	vga_putchar('\n');
	vga_update_cursor();

	while (1) 
	        asm volatile ("hlt");
}
