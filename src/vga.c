#include <kernel.h>

// vga(16): bg(4) fg(4) char(8)

void clear_screen() {
	struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;
	for (uint16_t i = 0; i < VGA_SIZE; i++)
		((uint16_t*)vga_info->base_addr)[i] = (vga_info->cur_fg_color << 4|vga_info->cur_bg_color) << 8|' ';
}

void set_blinking() {
	struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;
	uint16_t cur_cursor = vga_info->cur_cursor_row*VGA_W + vga_info->cur_cursor_col;
	((uint16_t*)vga_info->base_addr)[ cur_cursor ] = 0b1000000000000000;
}

void set_vga_fg_color(uint8_t co) {
	((struct vga_info_st*)VGA_INFO_ST)->cur_fg_color = co;
}

void set_vga_bg_color(uint8_t co) {
	((struct vga_info_st*)VGA_INFO_ST)->cur_bg_color = co;
}

void vga_putchar(uint8_t ch) {
	struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;
	uint16_t cur_cursor = vga_info->cur_cursor_row*VGA_W + vga_info->cur_cursor_col;
	((uint16_t*)vga_info->base_addr)[ cur_cursor ] = (vga_info->cur_fg_color << 4|vga_info->cur_bg_color) << 8|ch;
	vga_info->cur_cursor_col += 1;
	if (vga_info->cur_cursor_col > VGA_W) {
		vga_info->cur_cursor_col = 0;
		vga_info->cur_cursor_row += 1;
		if (vga_info->cur_cursor_row > VGA_H) {
			vga_info->cur_cursor_row = 0;
			clear_screen();
		}
	}
}

void vga_puthexchar(uint8_t ch) {
	uint8_t high = (ch >> 4) & 0x0f;
	uint8_t low = ch & 0x0f;
	vga_putchar( high < 10 ? high + '0' : (high - 10) + 'a' );
	vga_putchar( low < 10 ? low + '0' : (low - 10) + 'a' );
}

void vga_putword(uint16_t wo) {
	vga_puthexchar((wo >> 8) & 0xff);
	vga_puthexchar(wo & 0xff);
}

void vga_putdword(uint32_t dwo) {
	vga_putword((dwo >> 16) & 0xffff);
	vga_putword(dwo & 0xffff);
}

void vga_putqword(uint64_t qwo) {
	vga_putword((qwo >> 32) & 0xffffffff);
	vga_putword(qwo & 0xffffffff);
}

void init_vga() {
	struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;

	vga_info->base_addr = VGA_MEM;
	vga_info->cur_bg_color = VGA_BLUE;
	vga_info->cur_fg_color = VGA_WHITE;
	vga_info->cur_cursor_col = VGA_INIT_CURSOR_COL;
	vga_info->cur_cursor_row = VGA_INIT_CURSOR_ROW;

	clear_screen();
	set_blinking();
}

void putchar_idx(uint8_t c) {}
