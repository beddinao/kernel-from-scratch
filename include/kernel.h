#ifndef _KERNEL_H_
#define _KERNEL_H_

#define _04GB 0x100000000
#define _03GB 0xC0000000
#define _01MB 0x100000
#define _64KB 0x10000
#define _32KB 0x8000
#define _16KB 0x4000
#define _08KB 0x2000
#define _04KB 0x1000
#define _01KB 0x400

#define HIGH_HALF _03GB
#define MIN_RAM 0xa00000

#define VGA_W 80
#define VGA_H 25
#define VGA_SIZE (VGA_W*VGA_H)
#define VGA_MEM (0xb8000+HIGH_HALF)
#define VGA_INFO_ST (0x200000+HIGH_HALF)
#define VGA_BUFF0 (VGA_INFO_ST+_04KB)
#define VGA_BUFF1 (VGA_BUFF0+_04KB)
#define VGA_BUFF2 (VGA_BUFF1+_04KB)
#define VGA_BUFF3 (VGA_BUFF2+_04KB)
#define VGA_INIT_CURSOR_COL 0x7 
#define VGA_INIT_CURSOR_ROW 0x0
#define VGA_SCREENS 0b11 
#define CRTC_IDX_PORT 0x3d4
#define CRTC_DATA_PORT 0x3d5
#define CRTC_RG_STC 0xa
#define CRTC_RG_ENC 0xb
#define CRTC_RG_PH 0xe
#define CRTC_RG_PL 0xf

/*
(remote) gef➤  print _04KB*1024 - (((HIGH_HALF+_01MB*4)-4) - HIGH_HALF)
$22 = 0x4
(remote) gef➤  print (VGA_BUFF3 + VGA_SIZE) - HIGH_HALF
$27 = 0x304f40
*/

#define MB1_MAGIC_NM 0x2badb002

#define KSTACK_S _08KB

#define PMMF_S _04KB
#define PMMF_U 1
#define PMMF_F 0

#define RING_0 0b00
#define RING_1 0b01
#define RING_2 0b10
#define RING_3 0b11

#define GDT_BASE (0x800+HIGH_HALF)
#define GDT_D_S 8
#define GDT_D_C 7
#define GDT_D_CODE 0b1
#define GDT_D_DATA 0b0
#define GDT_D_CODE_RO 0b0
#define GDT_D_CODE_RX 0b1
#define GDT_D_DATA_RO 0b0
#define GDT_D_DATA_RW 0b1

#define IDT_D_S 8
#define IDT_D_C 256 
#define IDT_SIZE IDT_D_S*IDT_D_C

#define SHELL_PROMPT " *kfs> "
#define NEWLINE_SK '\n'
#define BACKSPACE_SK '\b'
#define LARROW_SK 0x13
#define RARROW_SK 0x12
#define CTRL_SK 0x11 

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

typedef char int8_t;
typedef short int int16_t;
typedef int int32_t;
typedef long int int64_t;

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;
extern uint32_t _kernel_stack;
extern uint32_t _boot_page_directory_start;
extern uint32_t _boot_page_directory_768;
extern uint32_t _boot_page_table_start;

struct idt_register {
	uint16_t size;
	uint32_t idt_addr;
} __attribute__((packed));

struct idt_gate_desc {
	uint16_t offset_low;
	uint16_t gdt_seg_selec;
	uint8_t reserved;
	uint8_t type_attr;
	uint16_t offset_high;
} __attribute__((packed));

struct gdt_register {
	uint16_t size;
	uint32_t gdt_addr;
} __attribute__((packed));

struct gdt_segment_desc {
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t access;
	uint8_t limit_high_flags;
	uint8_t base_high;
} __attribute__((packed));

// https://www.gnu.org/software/grub/manual/multiboot/multiboot.html#Boot-information-format

struct multiboot_info_st {
	uint32_t flags;
	uint32_t mem_lower;
	uint32_t mem_upper;
	uint32_t unrelevant_region[2];
	uint32_t mods_count;
	uint32_t mods_addr;
	uint32_t unrelevant_region2[4];
	uint32_t mmap_length;
        uint32_t mmap_addr;	
	uint32_t unrelevant_region3[3];
	uint32_t boot_loader_name;

	uint8_t unrelavant[20];

	uint64_t framebuffer_addr;
	uint32_t framebuffer_pitch;
	uint32_t framebuffer_width;
	uint32_t framebuffer_height;
	uint8_t framebuffer_bpp;
	uint8_t framebuffer_type;
	uint8_t framebuffer_red_field_position;
	uint8_t framebuffer_red_mask_size;
	uint8_t framebuffer_green_field_position;
	uint8_t framebuffer_green_mask_size;
	uint8_t framebuffer_blue_field_position;
	uint8_t framebuffer_blue_mask_size;
} __attribute__((packed));

struct multiboot_mmap_st {
	uint32_t size;
	uint32_t base_addr_low;
	uint32_t base_addr_high;
	uint32_t length_low;
	uint32_t length_high;
	uint32_t type;
} __attribute__((packed));

struct pmm_info_st {
	uint32_t size; 
	uint32_t frames_count;
	uint32_t alloced;
	uint32_t mm_ptr;
	uint32_t ram_size;
	uint8_t bitmap[];
} __attribute__((packed));

struct vga_screen_st {
	uint32_t base_addr;
	uint8_t cmd_line[VGA_W+1];
	uint8_t cmd_line_idx;
	uint8_t _color; // bg(4-7) fg(0-3)
	uint16_t _cursor; // row(8-15) col(0-7)
} __attribute__((packed));

struct vga_info_st {
	struct vga_screen_st _sc[0b100];
	uint8_t attr; // shell_on(3) _ctrl(2) _screen(0-1)
} __attribute__((packed));

enum vga_color {
	VGA_BLACK,
	VGA_BLUE,
	VGA_GREEN,
	VGA_CYAN,
	VGA_RED,
	VGA_MAGENTA,
	VGA_BROWN,
	VGA_LIGHT_GREY,
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
void vga_init();
void vga_putchar(uint8_t);
void vga_putnm(uint8_t*, uint32_t);
void vga_putstr(uint8_t*);
void vga_putstrlen(uint8_t*, uint8_t);
void vga_puthexnm(uint8_t);
void vga_puthexcnm(uint8_t*, uint8_t);
void vga_putword(uint8_t*, uint16_t);
void vga_putdword(uint8_t*, uint32_t);
void vga_set_fg_color(uint8_t);
void vga_set_bg_color(uint8_t);
void vga_enable_cursor();
void vga_disable_cursor();
void vga_update_cursor();
void vga_clear_screen(uint8_t);
void vga_reset_screen();
void vga_putcharsanitize(uint8_t);
void vga_putsizenm(uint8_t *, uint32_t);

// pmm.c
void pmm_init(uint32_t, uint32_t);
void pmm_free(uint32_t);
uint32_t pmm_alloc();
uint8_t pmm_isused(uint32_t);
void pmm_alloc_frame(uint32_t);
void pmm_alloc_chunk_len(uint32_t, uint32_t);
void pmm_alloc_chunk_eptr(uint32_t, uint32_t);
void pmm_free_chunk_len(uint32_t, uint32_t);
void pmm_free_chunk_eptr(uint32_t, uint32_t);

// vmm.c
void vmm_init();
void vmm_flush_tlb();
void vmm_set_pde(uint16_t, uint32_t);

// gdt.c
void gdt_init();

// idt.c
void idt_init();
uint8_t *idt_perm_offset();

// string.c
void kmemset(volatile uint8_t*, uint8_t, uint32_t);
void kmemsetw(volatile uint16_t*, uint16_t, uint32_t);
void kmemsetdw(volatile uint32_t*, uint32_t, uint32_t);
void kmemzero(volatile uint8_t*, uint32_t);
uint8_t kstrcmp(uint8_t*, uint8_t*);
uint16_t kstrlen(uint8_t*);
uint32_t ktonum(uint8_t*);

// shell.c
void kshell(uint8_t *, struct vga_info_st*);

// error.c
void panic(uint8_t*);

// print.c
void print_hdr(uint32_t);
void dump(uint32_t, uint32_t);

// multiboot.c
uint32_t inspect_mb_info(uint32_t, uint32_t);

#endif
