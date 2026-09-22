#include <kernel.h>


void panic(uint8_t *err) {
	vga_set_fg_color(VGA_WHITE);
	vga_set_bg_color(VGA_RED);
	vga_reset_screen();
	vga_disable_cursor();
	vga_putstr(err);
	asm volatile ("cli");
	while (1) asm volatile ("hlt");
}
