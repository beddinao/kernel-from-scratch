
#define VGA_W	80
#define VGA_H	25
#define VGA_MEM	0xb8000

// vga(16): bg(4) fg(4) char(8)

enum vga_color {
	VGA_BLACK,
	VGA_BLUE,
	VGA_GREEN,
	VGA_CYAN,
	VGA_RED,
	VGA_MAGENTA,
	VGA_BROWN,
	VGA_LIGH_GREY,
	VGA_DARK_GREY,
	VGA_LIGHT_BLUE,
	VGA_LIGHT_GREEN,
	VGA_LIGHT_CYAN,
	VGA_LIGHT_RED,
	VGA_LIGHT_MAGENTA,
	VGA_LIGHT_BROWN,
	VGA_WHITE
};

int kmain() {
	for (unsigned short i = 0; i < 2000; i++)
		((unsigned short*)VGA_MEM)[i] = (VGA_BLUE << 4|VGA_BLUE) << 8| ' ';

	((unsigned short*)VGA_MEM)[0] = (VGA_BLUE << 4|VGA_WHITE) << 8| '4';
	((unsigned short*)VGA_MEM)[1] = (VGA_BLUE << 4|VGA_WHITE) << 8| '2';
}
