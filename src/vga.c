#include <kernel.h>

// vga(16): bg(4) fg(4) char(8)

void vga_clear_screen() {
	volatile struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;
	for (uint16_t i = 0; i < VGA_SIZE; i++)
		((uint16_t*)vga_info->base_addr)[i] = (vga_info->cur_bg_color << 4|vga_info->cur_fg_color) << 8|' ';
}

void vga_enable_cursor() {
	asm volatile (

		"mov dx, 0x3d4\n"
		"mov al, 0xa\n"
		"out dx, al\n"

		"mov dx, 0x3d5\n"
		"mov al, 0b00001110\n"
		"out dx, al\n"

		"mov dx, 0x3d4\n"
		"mov al, 0xb\n"
		"out dx, al\n"

		"mov dx, 0x3d5\n"
		"mov al, 15\n"
		"out dx, al\n"

	);
}

void vga_disable_cursor() {
	asm volatile (
		"mov dx, 0x3d4\n"
		"mov al, 0xa\n"
		"out dx, al\n"
		"mov dx, 0x3d5\n"
		"mov al, 0b00100000\n"
		"out dx, al\n"
	);
}

void vga_update_cursor() {
	volatile struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;
	uint16_t cur_cursor = vga_info->cur_cursor_row*VGA_W + vga_info->cur_cursor_col;
	asm volatile (

		"mov dx, 0x3d4\n"
		"mov al, 0xe\n"
		"out dx, al\n"

		"mov dx, 0x3d5\n"
		"mov al, %b0\n"
		"out dx, al\n"

		"mov dx, 0x3d4\n"
		"mov al, 0xf\n"
		"out dx, al\n"

		"mov dx, 0x3d5\n"
		"mov al, %b1\n"
		"out dx, al\n"

		:
		: "q" (cur_cursor >> 8),
		  "q" (cur_cursor & 0xff)
		: "dx", "al"
	);
}

void set_vga_fg_color(uint8_t co) {
	((struct vga_info_st*)VGA_INFO_ST)->cur_fg_color = co;
}

void set_vga_bg_color(uint8_t co) {
	((struct vga_info_st*)VGA_INFO_ST)->cur_bg_color = co;
}

void vga_putchar(uint8_t ch) {
	volatile struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;
	if (ch == '\n') {
		vga_info->cur_cursor_col = 0;
		vga_info->cur_cursor_row += 1;
	}
	else {
		uint16_t cur_cursor = vga_info->cur_cursor_row*VGA_W + vga_info->cur_cursor_col;
		((uint16_t*)vga_info->base_addr)[ cur_cursor ] = (vga_info->cur_bg_color << 4|vga_info->cur_fg_color) << 8|ch;
		vga_info->cur_cursor_col += 1;
		if (vga_info->cur_cursor_col > VGA_W) {
			vga_info->cur_cursor_col = 0;
			vga_info->cur_cursor_row += 1;
		}
	}
	if (vga_info->cur_cursor_row > VGA_H) {
		vga_info->cur_cursor_row = 0;
		vga_clear_screen();
	}
}


void vga_putstr(uint8_t *p) {
	for (; *p != 0; p++)
		vga_putchar(*p);
}


void vga_puthexnm(uint8_t ch) {
	uint8_t high = (ch >> 4) & 0x0f;
	uint8_t low = ch & 0x0f;
	vga_putchar( high < 10 ? high + '0' : (high - 10) + 'a' );
	vga_putchar( low < 10 ? low + '0' : (low - 10) + 'a' );
}

void vga_putword(uint8_t *s, uint16_t wo) {
	vga_putstr(s);
	vga_puthexnm((wo >> 8) & 0xff);
	vga_puthexnm(wo & 0xff);
	vga_update_cursor();
}

void vga_putdword(uint8_t *s, uint32_t dwo) {
	vga_putstr(s);
	vga_puthexnm((dwo >> 24) & 0xff);
	vga_puthexnm((dwo >> 16) & 0xff);
	vga_puthexnm((dwo >> 8) & 0xff);
	vga_puthexnm(dwo & 0xff);
	vga_update_cursor();
}

void vga_init() {
	volatile struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;

	vga_info->base_addr = VGA_MEM;
	vga_info->cur_bg_color = VGA_BLUE;
	vga_info->cur_fg_color = VGA_WHITE;
	vga_info->cur_cursor_col = VGA_INIT_CURSOR_COL;
	vga_info->cur_cursor_row = VGA_INIT_CURSOR_ROW;

	vga_clear_screen();
	vga_enable_cursor();
}
