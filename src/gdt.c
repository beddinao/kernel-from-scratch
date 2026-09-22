#include <kernel.h>

void gdt_set_segment_access(volatile struct gdt_segment_desc* seg_desc, uint8_t type, uint8_t rw, uint8_t ring) {
	seg_desc->access = 0b10010000;
	seg_desc->access |= rw << 1;
	seg_desc->access |= type << 3;
	seg_desc->access |= ring << 5;
}

void gdt_set_segment_borders(volatile struct gdt_segment_desc* seg_desc, uint32_t base_addr, uint32_t limit_addr) {
	limit_addr /= PMMF_S;
	seg_desc->base_low = base_addr & 0xffff;
	seg_desc->base_mid = (base_addr >> 16) & 0xff;
	seg_desc->base_high = (base_addr >> 24) & 0xff;
	seg_desc->limit_low = limit_addr & 0xffff;
	seg_desc->limit_high_flags = seg_desc->limit_high_flags & 0xf0 | (limit_addr >> 16) & 0xf;
}

void gdt_dump_seg_registers() {
	uint16_t cs, ds, ss;

	asm volatile (
		"mov %0, cs\n"
		"mov %1, ds\n"
		"mov %2, ss\n"
		:
		: "m" (cs), "m" (ds), "m" (ss) 
	);

	vga_putstr("\nsegment_registers: ");
	vga_putword("cs:0x", cs);
	vga_putword(", ds:0x", ds);
	vga_putword(", ss:0x", ss);
}

void gdt_dump_seg_descriptors() {
	uint32_t *kernel_code_low = (uint32_t*)GDT_BASE + 2;
	uint32_t *kernel_code_high = kernel_code_low + 1;
	uint32_t *kernel_data_low = (uint32_t*)GDT_BASE + 4;
	uint32_t *kernel_data_high = kernel_data_low + 1;

	vga_putdword("\ngdt_base: 0x", GDT_BASE);
	vga_putdword("\ngdt_desc1 (kernel_code): 0x", *kernel_code_high); vga_putdword("", *kernel_code_low);
	vga_putdword("\ngdt_desc2 (kernel_data): 0x", *kernel_data_high); vga_putdword("", *kernel_data_low);
}

void gdt_init() {
	*((uint32_t*)GDT_BASE) = 0x0;
	*((uint32_t*)(GDT_BASE+1)) = 0x0;

	volatile struct gdt_segment_desc *kernel_code = (struct gdt_segment_desc*)(GDT_BASE + GDT_D_S);
	volatile struct gdt_segment_desc *kernel_data = (struct gdt_segment_desc*)(GDT_BASE + GDT_D_S*2);
	volatile struct gdt_segment_desc *kernel_stack = (struct gdt_segment_desc*)(GDT_BASE + GDT_D_S*3);
	volatile struct gdt_segment_desc *user_code = (struct gdt_segment_desc*)(GDT_BASE + GDT_D_S*4);
	volatile struct gdt_segment_desc *user_data = (struct gdt_segment_desc*)(GDT_BASE + GDT_D_S*5);
	volatile struct gdt_segment_desc *user_stack = (struct gdt_segment_desc*)(GDT_BASE + GDT_D_S*6);

	kernel_code->limit_high_flags = 0b11000000;
	kernel_data->limit_high_flags = 0b11000000;
	kernel_stack->limit_high_flags = 0b11000000;
	user_code->limit_high_flags = 0b11000000;
	user_data->limit_high_flags = 0b11000000;
	user_stack->limit_high_flags = 0b11000000;

	gdt_set_segment_borders(kernel_code, 0x00000000, 0xffffffff);
	gdt_set_segment_borders(kernel_data, 0x00000000, 0xffffffff);
	gdt_set_segment_borders(kernel_stack, 0x00000000, 0xffffffff);
	gdt_set_segment_borders(user_code, 0x00000000, 0xffffffff);
	gdt_set_segment_borders(user_data, 0x00000000, 0xffffffff);
	gdt_set_segment_borders(user_stack, 0x00000000, 0xffffffff);

	gdt_set_segment_access(kernel_code, GDT_D_CODE, GDT_D_CODE_RX, RING_0);
	gdt_set_segment_access(kernel_data, GDT_D_DATA, GDT_D_DATA_RW, RING_0);
	gdt_set_segment_access(kernel_stack, GDT_D_DATA, GDT_D_DATA_RW, RING_0);
	gdt_set_segment_access(user_code, GDT_D_CODE, GDT_D_CODE_RX, RING_3);
	gdt_set_segment_access(user_data, GDT_D_DATA, GDT_D_DATA_RW, RING_3);
	gdt_set_segment_access(user_stack, GDT_D_DATA, GDT_D_DATA_RW, RING_3);

	struct gdt_register gdtr = {(GDT_D_S * GDT_D_C) - 1, GDT_BASE};

	asm volatile (
		"lgdt %0\n"
		: : "m" (gdtr)
	);

	asm volatile (
		"mov ds, %w0\n"
		"mov es, %w0\n"
		"mov gs, %w0\n"
		"mov ss, %w1\n"
		:
		: "q" (0x10), "q" (0x18)
	);

	asm volatile (
		"jmp 0x8:1f\n"
		"1:\n"
	);

	gdt_dump_seg_descriptors();
	gdt_dump_seg_registers();
}
