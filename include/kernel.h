#ifndef _KERNEL_H_
#define _KERNEL_H_

#define VGA_W	80
#define VGA_H	25
#define VGA_SIZE	VGA_W*VGA_H
#define VGA_MEM	0xb8000
#define VGA_INFO_ST	0xA7D8C0
#define VGA_INIT_CURSOR_COL 0x0
#define VGA_INIT_CURSOR_ROW 0x0

#define MB1_MAGIC_NM 0x1badb002

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

typedef char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long int int64_t;

// https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format

struct multiboot_info_st {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t unrelevant_region[8]; // 256, 32b
	uint32_t mmap_length;
          uint32_t mmap_addr;	
	uint32_t unrelevant_region2[3]; // 96, 12b
	uint32_t boot_loader_name;
};

struct multiboot_mmap_st {
	uint32_t size;
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
};

struct vga_info_st {
	uint64_t base_addr;
	uint8_t cur_bg_color;
	uint8_t cur_fg_color;
	uint8_t cur_cursor_row;
	uint8_t cur_cursor_col;
};

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

// vga.c
void clear_screen();

#endif
