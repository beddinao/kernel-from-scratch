#include <kernel.h>

extern void isr_division_by_zero();
extern void isr_double_fault();
extern void isr_page_fault();
extern void isr_systimer();
extern void isr_keyboard();
extern void isr_dummy_error_code();
extern void isr_dummy();

uint8_t *idt_perm_offset() {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)(&_kernel_end);
	return (uint8_t*)pmm_info + sizeof(struct pmm_info_st) + pmm_info->size;
}

void idt_init() {
	uint8_t *idt_base_ptr = idt_perm_offset();
	uint8_t *idt_limit_ptr = idt_base_ptr + IDT_D_S*IDT_D_C;
	vga_putdword("\nidt_base: 0x", (uint32_t)idt_base_ptr);

	struct idt_gate_desc *gate_desc = (struct idt_gate_desc*)idt_base_ptr;

	for (uint16_t i = 0; i < 256; i++) {
		gate_desc->offset_low = (uint32_t)isr_dummy & 0xffff;
		gate_desc->offset_high = ((uint32_t)isr_dummy >> 16) & 0xffff;
		gate_desc->gdt_seg_selec = 0x8;
		gate_desc->reserved = 0x0;
		gate_desc->type_attr = 0b10010111;
		gate_desc += 1;
	}

	struct idt_register idtr = { IDT_D_S*IDT_D_S - 1, (uint32_t)idt_base_ptr };

	asm volatile (
		"lidt %0\n"
		//"sidt"
		:: "m" (idtr)
	);
}
