#include <kernel.h>

uint8_t *idt_perm_offset() {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)(&_kernel_end);
	return (uint8_t*)pmm_info + sizeof(struct pmm_info_st) + pmm_info->size;
}

void idt_init() {
	uint8_t *idt_base_ptr = idt_perm_offset();
	uint8_t *idt_limit_ptr = idt_base_ptr + IDT_D_S*IDT_D_C;


	




	vga_putdword("\nidt_base: 0x", (uint32_t)idt_base_ptr);
}
