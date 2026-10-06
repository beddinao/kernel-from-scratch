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

mop_res vmm_map_page(struct vmm_pde* pd, uint32_t vaddr, uint16_t flags) {
	uint16_t pde_idx = vaddr >> 22;
	uint16_t pte_idx = (vaddr & 0b1111111111000000000000) >> 12;
	struct vmm_pde *pde = &pd[pde_idx];

	if (!(pde->entry & 1)) {
		mop_res vmm_res = vmm_create_pde(pd, vaddr, flags >> 8);
		if (vmm_res != VMM_SUCCESS)
			return vmm_res;
	}

	struct vmm_pte *pte = (struct vmm_pte*)((1023 << 22) | (pde_idx << 12) | (pte_idx * 4));
	//struct vmm_pte *pte = (struct vmm_pte*)(pde->entry & 0b11111111111111111111000000000000);

	uint32_t frame = pmm_alloc();
	if (!frame) 
		return PMM_FAILURE;

	pte[pte_idx].entry = (frame & 0b11111111111111111111000000000000) | (flags & 0xff);
	//uint32_t vpt = (pde->entry & 0b11111111111111111111000000000000) - vmm_sys->vtop_off;
	//pde->entry = (vpt & 0b11111111111111111111000000000000) | vmm_sys->pte_flags;
	return VMM_SUCCESS;
}

mop_res vmm_map_pt(struct vmm_pde* pd, uint32_t vaddr, uint16_t flags) {
	uint16_t pde_idx = vaddr >> 22;
	struct vmm_pde *pde = &pd[pde_idx];

	if (!(pde->entry & 1)) {
		mop_res vmm_res = vmm_create_pde(pd, vaddr, flags >> 8);
		if (vmm_res != VMM_SUCCESS)
			return vmm_res;
	}

	struct vmm_pte *pte = (struct vmm_pte*)((1023 << 22) | (pde_idx << 12) | 0);
	//struct vmm_pte *pte = (struct vmm_pte*)(pde->entry & 0b11111111111111111111000000000000);

	for (uint32_t pte_idx = 0; pte_idx < 1024; pte_idx++) {
		uint32_t frame = pmm_alloc();
		if (!frame)
			return PMM_FAILURE;
		pte[pte_idx].entry = (frame & 0b11111111111111111111000000000000) | (flags & 0xff);
	}
	//uint32_t vpt = (pde->entry & 0b11111111111111111111000000000000) - vmm_sys->vtop_off;
	//pde->entry = (vpt & 0b11111111111111111111000000000000) | vmm_sys->pte_flags;
	vmm_flush_tlb();
	return VMM_SUCCESS;
}

mop_res vmm_map_range(struct vmm_pde *pd, uint32_t vaddr_s, uint32_t vaddr_e, uint16_t flags) {
	mop_res vmm_res = VMM_SUCCESS;
	for (; vaddr_s <= vaddr_e && vmm_res == VMM_SUCCESS; vaddr_s += _04KB)
		vmm_res = vmm_map_page(pd, vaddr_s, flags);
	vmm_flush_tlb();
	return vmm_res;
}

mop_res vmm_create_pde(struct vmm_pde* pd, uint32_t vaddr, uint8_t flags) {
	uint16_t pde_idx = vaddr >> 22;
	uint32_t pt_frame = pmm_alloc();

	if (!pt_frame)
		return PMM_FAILURE;
	
 	pd[pde_idx].entry = (pt_frame & 0b11111111111111111111000000000000 ) | flags;
	vmm_flush_tlb();
	return VMM_SUCCESS;
}

void vmm_init() {
	uint32_t p_pd = (uint32_t)&_boot_page_directory_start;
	uint32_t v_pd = p_pd + (uint32_t)&__high_half__;
	
	vmm_set_pde((struct vmm_pde*)v_pd, 1023, (p_pd & 0b11111111111111111111000000000000) | 0b11);

	/*
	struct vmm_sys_st vmm_sys = {
		(uint32_t)&_kernel_end + _04KB,
		(uint32_t)&__high_half__ + _04MB,
		(uint32_t)&__high_half__,
		0b11, 0b11, 1, 0
	};
	*/

	
	vmm_map_pt((struct vmm_pde*)v_pd, _03GB + _04MB, (0b11 << 8) | 0b11);
	kmemzero((uint8_t*)_03GB + _04MB, _04MB - 1);
	

	vmm_map_page((struct vmm_pde*)v_pd, _03GB + _04MB, (0b11 << 8 | 0b11));
	kmemzero((uint8_t*)_03GB + _04MB, _04KB);
}
