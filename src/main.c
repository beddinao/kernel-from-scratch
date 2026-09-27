#include <kernel.h>

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

	gdt_init();
	idt_init();

	((struct vga_info_st*)VGA_INFO_ST)->attr |= (1 << 3);
	vga_putchar('\n');
	vga_update_cursor();

	while (1) 
	        asm volatile ("hlt");
}
