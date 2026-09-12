#ifndef _KERNEL_H_
#define _KERNEL_H_

#define VGA_W	80
#define VGA_H	25
#define VGA_SIZE	VGA_W*VGA_H
#define VGA_MEM	0xb8000

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
void clear_screen(uint8_t);

#endif
