#include <kernel.h>

// vga(16): bg(4) fg(4) char(8)

void vga_clear_screen(uint8_t ch) {
	struct vga_info_st* vga_info = (struct vga_info_st*)VGA_INFO_ST;
	uint16_t *screen_buff = (uint16_t*)vga_info->_sc[vga_info->attr & 0b11].base_addr;
	uint8_t color = vga_info->_sc[vga_info->attr & 0b11]._color;
	for (uint16_t i = 0; i < VGA_SIZE; i++) 
		screen_buff[i] = color << 8 | (ch? ch: screen_buff[i]&0xff);
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
	uint16_t cursor = ((vga_info->_sc[vga_info->attr & 0b11]._cursor >> 8) & 0xff) * VGA_W
		+ (vga_info->_sc[vga_info->attr & 0b11]._cursor & 0xff);
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
			: "q" (cursor >> 8),
			"q" (cursor & 0xff)
				: "dx", "al"
					);
}

void vga_set_fg_color(uint8_t co) {
	((struct vga_info_st*)VGA_INFO_ST)->_sc[((struct vga_info_st*)VGA_INFO_ST)->attr & 0b11]._color |= co;
}

void vga_set_bg_color(uint8_t co) {
	((struct vga_info_st*)VGA_INFO_ST)->_sc[((struct vga_info_st*)VGA_INFO_ST)->attr & 0b11]._color |= (co << 4);
}

void vga_clear_line(volatile struct vga_info_st* vga_info) {
	struct vga_screen_st *screen = &(vga_info->_sc[vga_info->attr & 0b11]);
	for (uint8_t cur_col = 0; cur_col < VGA_W; cur_col++)
		((uint16_t*)screen->base_addr)[ ((screen->_cursor >> 8) & 0xff) * VGA_W + cur_col ] = screen->_color << 8 | ' ';
}

void vga_reset_screen() {
	struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;
	struct vga_screen_st *screen = &(vga_info->_sc[ vga_info->attr & 0b11 ]);

	vga_clear_screen(' ');

	screen->_cursor = VGA_INIT_CURSOR_ROW << 8 | VGA_INIT_CURSOR_COL;

	screen->cmd_line_idx = 0;
	kmemset(screen->cmd_line, sizeof(screen->cmd_line));
}

void vga_scroll(volatile struct vga_info_st* vga_info) {
	struct vga_screen_st *screen = &(vga_info->_sc[ vga_info->attr & 0b11 ]);

	//vga_info->_cursor_col = 0;
	//screen->_cursor &= 0b11110000;

	//vga_info->_cursor_row -= 1;
	screen->_cursor = ((screen->_cursor >> 8)-1) << 8 | 0;

	//vga_info->shell_enable = 0;
	vga_info->attr &= ~(1 << 3);
	
	for (uint8_t cur_row = 0; cur_row < VGA_H; cur_row++) {
		for (uint8_t i = 0; i < VGA_W; i++) {
			uint16_t idx = cur_row * VGA_W +i;
			((uint16_t*)screen->base_addr)[idx] = ((uint16_t*)screen->base_addr)[idx+VGA_W];
		}
	}

	vga_clear_line(vga_info);
	vga_putstr(SHELL_PROMPT);
	//vga_info->shell_enable = 1;
	vga_info->attr |= (1 << 3);
}

void vga_putchar(uint8_t ch) {
	volatile struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;
	struct vga_screen_st *screen = &(vga_info->_sc[ vga_info->attr & 0b11 ]);

	uint16_t cur_cursor;
	uint8_t line_start = 0;
	switch (ch) {
		case 0:
			return;
		case NEWLINE_SK:
			//vga_info->_cursor_col = 0;
			//screen->_cursor &= 0xff00;
			//vga_info->_cursor_row += 1;
			screen->_cursor = ((screen->_cursor >> 8)+1) << 8 | 0;

			if ((vga_info->attr >> 3) & 1) {
				//vga_info->shell_enable = 0;
				vga_info->attr &= ~(1 << 3);
				kshell(screen->cmd_line, vga_info);
				//vga_info->shell_enable = 1;
				vga_info->attr |= (1 << 3);
			}
			kmemset(screen->cmd_line, sizeof(screen->cmd_line));
			screen->cmd_line_idx = 0;
			if (((screen->_cursor >> 8) & 0xff) >= VGA_H)
				vga_scroll(vga_info);
			break;
		case BACKSPACE_SK:
			if ((vga_info->attr >> 3) & 1)
				line_start = kstrlen(SHELL_PROMPT);
			if (screen->_cursor & 0xff == line_start)
				break;

			screen->_cursor -= 1;
			//screen->_cursor = (screen->_cursor & 0xff00) | (screen->_cursor & 0xff) - 1;

			cur_cursor = ((screen->_cursor&0xff00)>>8) * VGA_W + (screen->_cursor&0xff);

			((uint16_t*)screen->base_addr)[ cur_cursor ] = screen->_color<<8|' ';

			if (screen->cmd_line_idx) screen->cmd_line_idx -= 1;
			screen->cmd_line[screen->cmd_line_idx] = 0;
			vga_update_cursor();
			break;
		case RARROW_SK:
			if (vga_info->attr & 0b100) {
				line_start = (vga_info->attr&0b11) + 1;
				vga_info->attr &= ~(0b11);
				vga_info->attr |= (line_start&0b11);
				vga_reset_screen();
				vga_update_cursor();
				break;
			}
		case LARROW_SK:
			if (vga_info->attr & 0b100) {
				line_start = (vga_info->attr&0b11) - 1;
				vga_info->attr &= ~(0b11);
				vga_info->attr |= (line_start&0b11);
				vga_reset_screen();
				vga_update_cursor();
				break;
			}
		default:
			cur_cursor = ((screen->_cursor&0xff00) >> 8)*VGA_W + (screen->_cursor&0xff);

			ch = (vga_info->attr & 0b100) ? (ch-32) : ch;

			((uint16_t*)screen->base_addr)[ cur_cursor ] = screen->_color<<8|ch;

			if ((vga_info->attr >> 3) & 1)
				screen->cmd_line[screen->cmd_line_idx++] = ch;

			screen->_cursor += 1;
			//screen->_cursor = (screen->_cursor & 0xff00) | (screen->_cursor & 0xff) + 1;

			if ((screen->_cursor & 0xff) > VGA_W)
				vga_putchar(NEWLINE_SK);

			break;
	}
}

void vga_putstrlen(uint8_t *p, uint8_t len) {
	for (; len; len--)
		vga_putchar(*p++);
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

	for (uint8_t i = 0; i < 0b11; i++) {
		kmemset(vga_info->_sc[i].cmd_line, sizeof(vga_info->_sc[i].cmd_line));
		vga_info->_sc[i].cmd_line_idx = 0;
		vga_info->_sc[i]._cursor = VGA_INIT_CURSOR_ROW << 8 | VGA_INIT_CURSOR_COL;
	}

	vga_info->attr = 0;

	vga_info->_sc[0b00]._color = VGA_BLUE << 4|VGA_WHITE;
	vga_info->_sc[0b01]._color = VGA_BLACK << 4|VGA_WHITE;
	vga_info->_sc[0b10]._color = VGA_LIGHT_GREY << 4|VGA_BLACK;
	vga_info->_sc[0b00].base_addr = VGA_MEM;
	vga_info->_sc[0b01].base_addr = VGA_MEM;
	vga_info->_sc[0b10].base_addr = VGA_MEM;

	vga_reset_screen();
	vga_enable_cursor();

}
