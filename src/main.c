#include <kernel.h>

uint32_t inspect_mb_info(uint32_t _magic, uint32_t mm_ptr) {
	volatile struct multiboot_info_st *mbd = (struct multiboot_info_st*)mm_ptr;
	uint32_t _tot_ram_size = 0;

	vga_putchar('\n');
	if (_magic != MB1_MAGIC_NM)
		panic("invalid multiboot protocol!");

	if ((mbd->flags >> 6) & 1) {

		uint8_t *ptr = (uint8_t*)mbd->mmap_addr;
		uint8_t *eptr = ptr + mbd->mmap_length;
		struct multiboot_mmap_st* mb_mmap;

		for (; ptr < eptr;) {

			mb_mmap = (struct multiboot_mmap_st*)ptr;

			_tot_ram_size += (mb_mmap->length_high << 32 | mb_mmap->length_low);
			if (mb_mmap->type == 1) {
			
				vga_putdword("  mem_region_start : 0x", mb_mmap->base_addr_low);
				vga_putdword(" length: 0x", mb_mmap->length_low);
				vga_putsizenm(" ", mb_mmap->length_low);
				vga_putstr(" type: FREE\n");
			}

			ptr += mb_mmap->size + 4;
		}

		vga_putdword("  RAM size         : 0x", _tot_ram_size);
		vga_putsizenm(" ", _tot_ram_size);
		if (_tot_ram_size < MIN_RAM)
			panic("insufficant memory at least 10Mb");
	}
	else panic("multiboot memory map is not present as requested!");
	return _tot_ram_size;
}

void print_hdr(uint32_t mm_ptr) {
	volatile struct multiboot_info_st *mbd = (struct multiboot_info_st*)mm_ptr;
	struct vga_info_st* vga_info = (struct vga_info_st*)VGA_INFO_ST;
	uint8_t old_color = vga_info->_sc[ vga_info->attr & 0b11 ]._color;
	//           "                                                                               "
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 3;
	vga_putstr("                  :::      ::::::::\n");
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 3;
	vga_putstr("                :+:      :+:    :+:\n");
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 11;
	vga_putstr("              +:+ +:+         +:+       ");
	if ((mbd->flags >> 9) & 1) {
		vga_putstr("BOSS: ");
		vga_putstr((uint8_t*)mbd->boot_loader_name);
	}
	vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 15;
	vga_putdword("            +#+  +:+       +#+          kernel_start: 0x", (uint32_t)&_kernel_start); vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 15;
	vga_putdword("          +#+#+#+#+#+   +#+             kernel_end  : 0x", (uint32_t)&_kernel_end);
	vga_putsizenm(" ", (uint32_t)&_kernel_end-(uint32_t)&_kernel_start); vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 11;
	vga_putdword("               #+#    #+#               stack_top   : 0x", (uint32_t)&_kernel_stack); vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color&0b11110000 | 11;
	vga_putdword("              ###   ########            stack_bottom: 0x", (uint32_t)&_kernel_stack-KSTACK_S); 
	vga_putsizenm(" ", KSTACK_S); vga_putchar('\n');
	vga_info->_sc[ vga_info->attr & 0b11 ]._color = old_color;
}

int kmain(uint32_t _magic, uint32_t mm_ptr) {

	vga_init();

	print_hdr(mm_ptr);
	pmm_init(mm_ptr, inspect_mb_info(_magic, mm_ptr));
	vmm_init();

	gdt_init();
	idt_init();

	((struct vga_info_st*)VGA_INFO_ST)->attr |= (1 << 3);
	vga_putchar('\n');
	vga_update_cursor();

	while (1) 
	        asm volatile ("hlt");
}
