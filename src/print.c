#include <kernel.h>

void dump(uint32_t st, uint32_t en) {
	uint32_t dw_ptr, dw;
	uint16_t *cur_cursor;
	for (; st <= en; st += 4) {
		vga_putdword("0x", st); vga_putchar(' ');
		for (dw_ptr = st; dw_ptr < st+4 && dw_ptr <= en; dw_ptr++) {
			dw = *(uint32_t*)dw_ptr;
			vga_putchar(' '); vga_puthexnm( dw >> 24 );
			vga_putchar(' '); vga_puthexnm( dw >> 16 );
			vga_putchar(' '); vga_puthexnm( dw >> 8 );
			vga_putchar(' '); vga_puthexnm( dw );
			vga_putchar(' ');
		}

		cur_cursor = &((struct vga_info_st*)VGA_INFO_ST)->_sc[ ((struct vga_info_st*)VGA_INFO_ST)->attr & 0b11 ]._cursor;
		*cur_cursor = *cur_cursor & 0xff00 | (VGA_W - 16);

		for (dw_ptr = st; dw_ptr < st+4 && dw_ptr <= en; dw_ptr++) {
			dw = *(uint32_t*)dw_ptr;
			vga_putcharsanitize(dw >> 24);
			vga_putcharsanitize(dw >> 16);
			vga_putcharsanitize(dw >> 8);
			vga_putcharsanitize(dw);
		}
		vga_putchar('\n');
	}
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
		vga_putstr((uint8_t*)(mbd->boot_loader_name+HIGH_HALF));
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

