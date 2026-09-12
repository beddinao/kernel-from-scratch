#include <kernel.h>

// vga(16): bg(4) fg(4) char(8)

void clear_screen(uint8_t bg) {
	for (uint16_t i = 0; i < VGA_SIZE; i++)
		((uint16_t*)VGA_MEM)[i] = (bg << 4|bg) << 8|' ';
}


void putchar_idx(uint8_t c) {}
