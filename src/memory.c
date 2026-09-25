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
