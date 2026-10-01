#include <kernel.h>

void vmm_init() {
	uint32_t *pd = (uint32_t*)((uint32_t)&_boot_page_directory_start + HIGH_HALF);

	uint16_t idx = GDT_BASE % 0x400000;


	/*
	pd[idx] = 

	pd[0] = 0; */ 
}
