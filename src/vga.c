#include <kernel.h>

// vga(16): bg(4) fg(4) char(8)

void vga_puttoscreen(struct vga_info_st *vga_info, uint16_t pos, uint16_t ch) {
	volatile uint16_t *screen_buff = (uint16_t*)vga_info->_sc[ vga_info->attr && 0b11 ].base_addr;
	volatile uint16_t *vga_buff = (uint16_t*)VGA_MEM;

	screen_buff[pos] = ch;
	vga_buff[pos] = ch;
}

void vga_clear_screen(uint8_t ch) {
	struct vga_info_st* vga_info = (struct vga_info_st*)&_vga_info_st;
	uint16_t *screen_buff = (uint16_t*)vga_info->_sc[ vga_info->attr & 0b11 ].base_addr;
	uint8_t color = vga_info->_sc[vga_info->attr & 0b11]._color;
	for (uint16_t i = 0; i < VGA_SIZE; i++)
		vga_puttoscreen(vga_info, i, color << 8 | (ch? ch: screen_buff[i]&0xff));
}

void vga_copy_screen(uint16_t *dest, uint16_t *src) {
	for (uint16_t i = 0; i < VGA_SIZE; i++)
		dest[i] = src[i];
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
	struct vga_info_st *vga_info = (struct vga_info_st*)&_vga_info_st;
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
	struct vga_info_st* vga_info = (struct vga_info_st*)&_vga_info_st;
	vga_info->_sc[vga_info->attr & 0b11]._color &= 0b11110000;
	vga_info->_sc[vga_info->attr & 0b11]._color |= co;
}

void vga_set_bg_color(uint8_t co) {
	struct vga_info_st* vga_info = (struct vga_info_st*)&_vga_info_st;
	vga_info->_sc[vga_info->attr & 0b11]._color &= 0b00001111;
	vga_info->_sc[vga_info->attr & 0b11]._color |= (co << 4);
}

void vga_clear_line(struct vga_info_st* vga_info) {
	struct vga_screen_st *screen = &(vga_info->_sc[vga_info->attr & 0b11]);
	for (uint8_t cur_col = 0; cur_col < VGA_W; cur_col++)
		vga_puttoscreen(vga_info, ((screen->_cursor >> 8) & 0xff) * VGA_W + cur_col, screen->_color << 8 | ' ');
}

void vga_reset_screen() {
	struct vga_info_st *vga_info = (struct vga_info_st*)&_vga_info_st;
	struct vga_screen_st *screen = &(vga_info->_sc[ vga_info->attr & 0b11 ]);

	vga_clear_screen(' ');

	screen->_cursor = VGA_INIT_CURSOR_ROW << 8 | 0;

	screen->cmd_line_idx = 0;
	kmemzero(screen->cmd_line, sizeof(screen->cmd_line));
}

void vga_scroll(struct vga_info_st* vga_info) {
	struct vga_screen_st *screen = &(vga_info->_sc[ vga_info->attr & 0b11 ]);
	uint16_t* screen_buff = (uint16_t*)screen->base_addr;

	screen->_cursor = ((screen->_cursor >> 8)-1) << 8 | 0;

	uint16_t idx;
	for (uint8_t cur_row = 0; cur_row < VGA_H; cur_row++) {
		for (uint8_t i = 0; i < VGA_W; i++) {
			idx = cur_row * VGA_W +i;
			vga_puttoscreen(vga_info, idx, screen_buff[idx+VGA_W]);
		}
	}

	vga_clear_line(vga_info);
}

void vga_putchar(uint8_t ch) {
	struct vga_info_st *vga_info = (struct vga_info_st*)&_vga_info_st;
	struct vga_screen_st *screen = &(vga_info->_sc[ vga_info->attr & 0b11 ]);

	uint16_t cur_cursor;
	uint8_t line_start = 0;
	switch (ch) {
		case 0:
			return;
		case NEWLINE_SK:
			screen->_cursor = ((screen->_cursor >> 8)+1) << 8 | 0;

			if (((screen->_cursor >> 8) & 0xff) >= VGA_H)
				vga_scroll(vga_info);

			if ((vga_info->attr >> 3) & 1) {
				vga_info->attr &= ~(1 << 3);
				kshell(screen->cmd_line, vga_info);
				vga_info->attr |= (1 << 3);
			}
			kmemzero(screen->cmd_line, sizeof(screen->cmd_line));
			screen->cmd_line_idx = 0;
			break;
		case BACKSPACE_SK:
			if ((vga_info->attr >> 3) & 1)
				line_start = kstrlen(SHELL_PROMPT);
			if ((screen->_cursor & 0xff) == line_start)
				break;

			screen->_cursor -= 1;
			//screen->_cursor = (screen->_cursor & 0xff00) | (screen->_cursor & 0xff) - 1;

			cur_cursor = ((screen->_cursor&0xff00)>>8) * VGA_W + (screen->_cursor&0xff);

			vga_puttoscreen(vga_info, cur_cursor, screen->_color<<8|' ');

			if (screen->cmd_line_idx) screen->cmd_line_idx -= 1;
			screen->cmd_line[screen->cmd_line_idx] = 0;
			vga_update_cursor();
			break;
		case RARROW_SK:
			if (vga_info->attr & 0b100) {
				line_start = (vga_info->attr&0b11) + 1;
				vga_info->attr &= ~(0b11);
				vga_info->attr |= (line_start&0b11);
				vga_copy_screen((uint16_t*)VGA_MEM, (uint16_t*)vga_info->_sc[ vga_info->attr & 0b11 ].base_addr);
			}
			else {
				if ((screen->_cursor & 0xff) - kstrlen(SHELL_PROMPT) + 1 > screen->cmd_line_idx)
					break;
				screen->_cursor += 1;
				screen->cmd_line_idx += 1;
			}
			vga_update_cursor();
			break;
		case LARROW_SK:
			if (vga_info->attr & 0b100) {
				line_start = (vga_info->attr&0b11) - 1;
				vga_info->attr &= ~(0b11);
				vga_info->attr |= (line_start&0b11);
				vga_copy_screen((uint16_t*)VGA_MEM, (uint16_t*)vga_info->_sc[ vga_info->attr & 0b11 ].base_addr);
				vga_update_cursor();
			}
			else {
				if ((vga_info->attr >> 3) & 1)
					line_start = kstrlen(SHELL_PROMPT);
				if ((screen->_cursor & 0xff) == line_start)
					break;

				screen->_cursor -= 1;
				screen->cmd_line_idx -= 1;
			}
			vga_update_cursor();
			break;
		default:
			if ((screen->_cursor & 0xff) >= VGA_W-1)
				break;

			cur_cursor = ((screen->_cursor&0xff00) >> 8)*VGA_W + (screen->_cursor&0xff);
			ch = ((vga_info->attr & 0b100) && ch <= 'z' && ch >= 'a') ? (ch-32) : ch;

			vga_puttoscreen(vga_info, cur_cursor, screen->_color<<8|ch);

			if ((vga_info->attr >> 3) & 1) 
				screen->cmd_line[screen->cmd_line_idx++] = ch;

			screen->_cursor += 1;
			//screen->_cursor = (screen->_cursor & 0xff00) | (screen->_cursor & 0xff) + 1;

			if ((screen->_cursor & 0xff) > VGA_W) {
				vga_info->attr &= ~(1<<3);
				vga_putchar(NEWLINE_SK);
				vga_info->attr |=  (1<<3);
			}

			break;
	}
}

void vga_putcharsanitize(uint8_t ch) {
	if ((ch <= '9' && ch >= '0')
		|| (ch >= 'a' && ch <= 'z')
		|| (ch >= 'A' && ch <= 'Z'))
		vga_putchar(ch);
	else vga_putchar('.');
}

void vga_putstrlen(uint8_t *p, uint8_t len) {
	for (; len; len--)
		vga_putchar(*p++);
}

void vga_putstr(uint8_t *p) {
	for (; *p != 0; p++)
		vga_putchar(*p);
}

void vga_putnm(uint8_t *str, uint32_t nm) {
	vga_putstr(str);
	if (!nm) {
		vga_putchar('0');
		return;
	}
	uint32_t di = 1;
	while (di < nm / 10) di *= 10;
	for (; di; di /= 10) {
		vga_putchar('0' + (nm/di));
		nm %= di;
	}
}

void vga_putsizenm(uint8_t* str, uint32_t nm) {
	uint8_t ch = 'K';

	if (nm >= _01GB) {
		nm /= _01GB;
		ch = 'G';
	}
	else if (nm >= _01MB) {
		nm /= _01MB;
		ch += 2;
	}
	else if (nm >= _01KB) 
		nm /= _01KB;
	else	ch = 0;

	vga_putnm(str, nm);
	vga_putchar(ch);
	vga_putchar('B');
}

void vga_puthexnm(uint8_t ch) {
	uint8_t high = (ch >> 4) & 0x0f;
	uint8_t low = ch & 0x0f;
	vga_putchar( high < 10 ? high + '0' : (high - 10) + 'a' );
	vga_putchar( low < 10 ? low + '0' : (low - 10) + 'a' );
}

void vga_puthexcnm(uint8_t *s, uint8_t ch) {
	vga_putstr(s);
	vga_puthexnm(ch);
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

void vga_putpmttoscreen(volatile uint16_t *buff, uint8_t co, uint8_t *str, uint16_t size) {
	for (uint16_t i = 0; i < size; i++)
		buff[i] = co << 8 | str[i];
}

void vga_init() {
	struct vga_info_st *vga_info = (struct vga_info_st*)&_vga_info_st;

	vga_info->_sc[0b00]._color = VGA_BLUE << 4|VGA_WHITE;
	vga_info->_sc[0b01]._color = VGA_BLACK << 4|VGA_WHITE;
	vga_info->_sc[0b10]._color = VGA_LIGHT_GREY << 4|VGA_BLACK;
	vga_info->_sc[0b11]._color = VGA_RED << 4|VGA_WHITE;

	vga_info->_sc[0b00].base_addr = (uint32_t)&_vga_buff_0;
	vga_info->_sc[0b01].base_addr = (uint32_t)&_vga_buff_1;
	vga_info->_sc[0b10].base_addr = (uint32_t)&_vga_buff_2;
	vga_info->_sc[0b11].base_addr = (uint32_t)&_vga_buff_3;

	uint16_t pmtlen = kstrlen(SHELL_PROMPT);
	struct vga_screen_st *screen;
	volatile uint16_t *screen_buff;
	volatile uint16_t *vga_buff = (uint16_t*)(VGA_MEM);

	for (uint8_t i = 0; i < 0b100; i++) {
		screen = &vga_info->_sc[i];
		screen_buff = (uint16_t*)screen->base_addr;

		kmemzero(screen->cmd_line, sizeof(screen->cmd_line));
		screen->cmd_line_idx = 0;
		screen->_cursor = VGA_INIT_CURSOR_ROW << 8 | VGA_INIT_CURSOR_COL;

		kmemsetw(screen_buff, screen->_color<<8|' ', VGA_SIZE);
		if (!i) kmemsetw(vga_buff, screen->_color<<8|' ', VGA_SIZE);
		else vga_putpmttoscreen(screen_buff, screen->_color, SHELL_PROMPT, pmtlen);
	}

	vga_info->attr = 0;
	vga_info->_sc[0b00]._cursor = 0;
	vga_enable_cursor();
}
