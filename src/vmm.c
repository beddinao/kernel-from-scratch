#include <kernel.h>

void vmm_flush_tlb() {
	asm volatile (
		"mov eax, cr3\n"
		"mov cr3, eax"
	);
}

void vmm_set_pde(uint16_t idx, uint32_t val) {
	uint32_t *pd = (uint32_t*)((uint32_t)&_boot_page_directory_start + HIGH_HALF);
	pd[idx] = val;
	vmm_flush_tlb();
}

void vmm_init() {
	uint32_t *pd = (uint32_t*)((uint32_t)&_boot_page_directory_start + HIGH_HALF);

	uint16_t idx = GDT_BASE % 0x400000;
}
