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

void vmm_map_page(struct vmm_pde* pd, uint32_t vaddr, struct vmm_sys_st *vmm_sys) {
	uint16_t pde_idx = vaddr >> 22;
	uint16_t pte_idx = (vaddr & 0b1111111111000000000000) >> 12;
	struct vmm_pde *pde = &pd[pde_idx];

	if (!(pde->entry & 1))
		vmm_create_pde(pd, vaddr, vmm_sys);

	if (!vmm_sys->op)
		return;

	struct vmm_pte *pte = (struct vmm_pte*)(pde->entry & 0b11111111111111111111000000000000);
	uint32_t frame = pmm_alloc();

	if (!frame) {
		pte[pte_idx].entry = 0;
		vmm_sys->op = 0;
		return;
	}

	pte[pte_idx].entry = (frame & 0b11111111111111111111000000000000) | vmm_sys->pte_flags;
	uint32_t vpt = (pde->entry & 0b11111111111111111111000000000000) - vmm_sys->vtop_off;
	pde->entry = (vpt & 0b11111111111111111111000000000000) | vmm_sys->pte_flags;
}

void vmm_map_pt(struct vmm_pde* pd, uint32_t vaddr, struct vmm_sys_st *vmm_sys) {
	uint16_t pde_idx = vaddr >> 22;
	struct vmm_pde *pde = &pd[pde_idx];

	if (!(pde->entry & 1))
		vmm_create_pde(pd, vaddr, vmm_sys);
	if (!vmm_sys->op)
		return;

	struct vmm_pte *pte = (struct vmm_pte*)(pde->entry & 0b11111111111111111111000000000000);

	for (uint32_t pte_idx = 0; pte_idx < 1024; pte_idx++) {
		uint32_t frame = pmm_alloc();
		if (!frame) {
			pte[pte_idx].entry = 0;
			vmm_sys->op = 0;
			return;
		}
		//if (!pte_idx || pte_idx == 1023) vga_putdword("\nallc_frame: 0x", frame);
		pte[pte_idx].entry = (frame & 0b11111111111111111111000000000000) | vmm_sys->pte_flags;
	}
	uint32_t vpt = (pde->entry & 0b11111111111111111111000000000000) - vmm_sys->vtop_off;
	pde->entry = (vpt & 0b11111111111111111111000000000000) | vmm_sys->pte_flags;
}

void vmm_map_range(struct vmm_pde *pd, uint32_t vaddr_s, uint32_t vaddr_e, struct vmm_sys_st *vmm_sys) {
	for (; vaddr_s <= vaddr_e; vaddr_s += _04KB)
		vmm_map_page(pd, vaddr_s, vmm_sys);
}

void vmm_create_pde(struct vmm_pde* pd, uint32_t vaddr, struct vmm_sys_st *vmm_sys) {
	uint32_t avail_sys_sp = vmm_sys->fe - vmm_sys->fs - (vmm_sys->count*_04KB);

	if (avail_sys_sp < _04KB) {
		vmm_sys->op = 0;
		return;
	}

	uint16_t pde_idx = vaddr >> 22;
	uint32_t pt_frame = pmm_alloc_frange(vmm_sys);

	if (!vmm_sys->op) {
		pd[pde_idx].entry = 0;
		return;
	}

	kmemzero((uint8_t*)pt_frame, _04KB);
 	pd[pde_idx].entry = (pt_frame & 0b11111111111111111111000000000000 ) | vmm_sys->pde_flags;
}

uint32_t vmm_create_dir(struct vmm_sys_st *vmm_sys) {
	uint32_t pd_frame = pmm_alloc_frange(vmm_sys);

	if (!vmm_sys->op)
		return 0;

	kmemzero((uint8_t*)pd_frame, _04KB);
	return pd_frame;
}

uint32_t vmm_addr_vtop(struct vmm_pde* pd, uint32_t vaddr) {
	uint16_t pde_idx = vaddr >> 22;
	uint16_t pte_idx = (vaddr & 0b1111111111000000000000) >> 12;
	struct vmm_pde* pde = &pd[pde_idx];

	
	if (!pde || !pde->entry)
		return 0;

	struct vmm_pte *pte = (struct vmm_pte*)(pde->entry & 0b11111111111111111111000000000000);

	if (!pte || !pte[pte_idx].entry)
		return 0;

	return (pte[pte_idx].entry & 0b11111111111111111111000000000000) + (vaddr & 0b111111111111);
}

void vmm_init() {
	struct vmm_pde *boot_pd = (struct vmm_pde*)((uint32_t)&_boot_page_directory_start + (uint32_t)&__high_half__);

	struct vmm_sys_st vmm_sys = {
		(uint32_t)&_kernel_end + _04KB,
		(uint32_t)&__high_half__ + _04MB,
		(uint32_t)&__high_half__,
		0b11, 0b11, 1, 0
	};

	vmm_map_pt(boot_pd, _01GB, &vmm_sys);
	vmm_flush_tlb();
	kmemzero((uint8_t*)_01GB, _04MB - 1);
}
