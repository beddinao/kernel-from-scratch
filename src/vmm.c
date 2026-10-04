#include <kernel.h>

void vmm_flush_tlb() {
	asm volatile (
		"mov eax, cr3\n"
		"mov cr3, eax"
	);
}

void vmm_load_pd(uint32_t pd) {
	asm volatile (
		"mov eax, %0\n"
		"mov cr3, eax"
		:: "m" (pd)
	);
}

void vmm_set_pde(struct vmm_pde *pd, uint16_t idx, uint32_t val) {
	pd[idx].entry = val;
	vmm_flush_tlb();
}

void vmm_map_page(struct vmm_pde* pd, uint32_t vaddr, uint8_t flags) {
	uint16_t pde_idx = vaddr >> 22;
	uint16_t pte_idx = (vaddr & 0b1111111111000000000000) >> 12;

	struct vmm_pde *pde = &pd[pde_idx];
	if (!(pde->entry & 1))
		vmm_create_pde(pd, vaddr, flags);

	struct vmm_pte *pte = (struct vmm_pte*)(pde->entry >> 12);

	if (pte->entry & 1) {
		pmm_free(pte->entry >> 12);
		vga_putdword("\n freed frame: 0x", pte->entry >> 12);
		pte->entry = 0;
	}

	uint32_t frame = pmm_alloc();
	vga_putdword("\n alloced frame: 0x", frame);
	pte[pte_idx].entry = (frame << 12) | flags;
}

void vmm_map_range(struct vmm_pde* pd, uint32_t vaddr_min, uint32_t vaddr_max, uint8_t flags) {
	for (uint32_t vaddr = vaddr_min; vaddr <= vaddr_max; vaddr += _04KB)
		vmm_map_page(pd, vaddr, flags);
}

void vmm_create_pde(struct vmm_pde* pd, uint32_t vaddr, uint8_t flags) {
	uint16_t pde_idx = vaddr >> 22;
	uint32_t pt_frame = pmm_alloc();
	//kmemzero((uint8_t*)pt_frame, _04KB); // very fucking tricky
	pd[pde_idx].entry = (pt_frame << 12) | flags;
}

uint32_t vmm_create_dir(uint8_t flags) {
	uint32_t pd_frame = pmm_alloc();
	return (pd_frame << 12 | flags);
}

uint32_t vmm_addr_vtop(struct vmm_pde* pd, uint32_t vaddr) {
	uint16_t pde_idx = vaddr >> 22;
	uint16_t pte_idx = (vaddr & 0b1111111111000000000000) >> 12;
	struct vmm_pte *pte = (struct vmm_pte*)(pd[pde_idx].entry >> 12);
	return (pte[pte_idx].entry >> 12) + (vaddr & 0b111111111111);
}

void vmm_init() {
	struct vmm_pde *boot_pd = (struct vmm_pde*)((uint32_t)&_boot_page_directory_start + (uint32_t)&__high_half__);

	/*vga_putdword("\nboot_pd: 0x", (uint32_t)boot_pd);
	vga_putdword(" == 0x", (uint32_t)&_boot_page_directory_start + (uint32_t)&__high_half__);
	vga_putdword(" , 768: 0x", (uint32_t)(&boot_pd[768]));
	vga_putdword(" == 0x", (uint32_t)&_boot_page_directory_768);*/

	vmm_map_page(boot_pd, _01GB, 0b11);
	
	vga_putdword("\n0x", _01GB);
	vga_putdword(" == 0x", vmm_addr_vtop(boot_pd, _01GB));

	vmm_map_page(boot_pd, _01GB, 0b11);
	
	vga_putdword("\n0x", _01GB);
	vga_putdword(" == 0x", vmm_addr_vtop(boot_pd, _01GB));
}
