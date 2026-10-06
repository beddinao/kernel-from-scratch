#include <kernel.h>

void pmm_alloc_frame(uint32_t addr) {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_pmm_info_st;
	uint32_t frame_idx = (addr / PMMF_S);
	pmm_info->bitmap[ frame_idx / 8 ] |= 1 << (frame_idx % 8); // 11[0]11|00[1]00
	pmm_info->alloced += 1;
}

uint32_t pmm_alloc_frange(struct vmm_sys_st *vmm_sys) {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_pmm_info_st;
	vmm_sys->op = 0;

	if (pmm_info->alloced >= pmm_info->frames_count) 
		return 0;

	uint32_t sframe = (vmm_sys->fs-vmm_sys->vtop_off) / PMMF_S;
	uint32_t eframe = (vmm_sys->fe-vmm_sys->vtop_off) / PMMF_S;

	uint32_t frame_byte_idx;
	uint8_t frame_bit_idx;

	for (; sframe <= eframe; sframe++) {

		frame_byte_idx = sframe / 8;
		frame_bit_idx = sframe % 8;

		if (!((pmm_info->bitmap[ frame_byte_idx ] >> frame_bit_idx) & 1)) {

			pmm_info->bitmap[ frame_byte_idx ] |= 1 << frame_bit_idx;

			vmm_sys->op = 1;
			vmm_sys->count += 1;
			pmm_info->alloced += 1;

			vga_putnm("\n  VMM PT frame ", vmm_sys->count);
			vga_putdword("   : PHYS 0x", sframe * PMMF_S);
			vga_putdword(", VIRT 0x", sframe * PMMF_S + vmm_sys->vtop_off);
			vga_putnm("; free PT frames: ",
				(vmm_sys->fe-vmm_sys->fs - (vmm_sys->count*_04KB)) / PMMF_S);

			return sframe * PMMF_S + vmm_sys->vtop_off;
		}
	}

	return 0;
}

void pmm_free(uint32_t addr) {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_pmm_info_st;
	uint32_t frame_idx = (addr / PMMF_S);
	pmm_info->bitmap[ frame_idx / 8 ] &= ~(1 << (frame_idx % 8)); // 00[1]00 & 11[0]11 
	pmm_info->alloced -= 1;
}

uint32_t pmm_alloc() {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_pmm_info_st;
	uint8_t frame_bit_idx;
	uint32_t frame_byte_idx;

	if (pmm_info->alloced >= pmm_info->frames_count)
		return 0;

	for (uint32_t frame_idx = 0; frame_idx < pmm_info->frames_count; frame_idx++) {
		frame_bit_idx = frame_idx % 8;
		frame_byte_idx = frame_idx / 8;
		
		if (! ((pmm_info->bitmap[ frame_byte_idx ] >> frame_bit_idx) & 1) ) {
			pmm_info->bitmap[ frame_byte_idx ] |= 1 << frame_bit_idx;
			pmm_info->alloced += 1;
			return frame_idx * PMMF_S;
		}

	}
	return 0;
}

uint8_t pmm_isused(uint32_t addr) {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_pmm_info_st;
	uint32_t frame_idx = (addr / PMMF_S);
	return (pmm_info->bitmap[ frame_idx / 8 ] >> (frame_idx % 8)) & 1;
}

void pmm_free_chunk_eptr(uint32_t base_addr, uint32_t eptr) {
	for (; base_addr < eptr; base_addr += PMMF_S)
		pmm_free(base_addr);
}

void pmm_free_chunk_len(uint32_t base_addr, uint32_t length) {
	pmm_free_chunk_eptr(base_addr, base_addr + length);
}

void pmm_alloc_chunk_eptr(uint32_t base_addr, uint32_t eptr) {
	for (; base_addr < eptr; base_addr += PMMF_S)
		pmm_alloc_frame(base_addr);
}

void pmm_alloc_chunk_len(uint32_t base_addr, uint32_t length) {
	pmm_alloc_chunk_eptr(base_addr, base_addr + length);
}

void pmm_dump_mmap(uint32_t sidx, uint32_t eidx) {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_pmm_info_st;
	vga_putdword("\npmm_mmap frames dump from 0x", sidx);
	vga_putdword(" to 0x", eidx); vga_putstr(": ");
	for (; sidx < eidx; sidx++) 
		vga_putchar( ( pmm_info->bitmap[sidx/8] & (1 << sidx%8) ) ? '1' : '0');
	vga_putchar('\n');
}

void pmm_alloc_reserved_system() {
	// https://wiki.osdev.org/Memory_Map_(x86)
	pmm_alloc_chunk_eptr(0x00000000, 0x000003FF); // real mode IVT
	pmm_alloc_chunk_eptr(0x00000400, 0x000004FF); // bda
	pmm_alloc_chunk_eptr(0x00080000, 0x0009FFFF); // ebda
	pmm_alloc_chunk_eptr(0x000A0000, 0x000BFFFF); // video display
	pmm_alloc_chunk_eptr(0x000C0000, 0x000C7FFF); // video BIOS
	pmm_alloc_chunk_eptr(0x000C8000, 0x000EFFFF); // BIOS expansion
	pmm_alloc_chunk_eptr(0x000F0000, 0x000FFFFF); // motherboard BIOS
	pmm_alloc_chunk_eptr(0x00000000, 0x00100000);
}

void pmm_alloc_reserved_kernel() {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_pmm_info_st;
	pmm_alloc_chunk_eptr(
			(uint32_t)&_kernel_start - (uint32_t)&__high_half__,
			(uint32_t)&_kernel_end - (uint32_t)&__high_half__); // kernel itself
	//pmm_alloc_chunk_len((uint32_t)&_kernel_end, pmm_info->size + sizeof(struct pmm_info_st)); NOTE: NOW INSIDE .bss
	//pmm_alloc_chunk_len(VGA_INFO_ST, sizeof(struct vga_info_st)); NOTE: NOW INSIDE .bss
	//pmm_alloc_chunk_len(VGA_BUFF0, VGA_SIZE*3); NOTE: NOW INSIDE .bss
	//pmm_alloc_chunk_len(VGA_BUFF1, VGA_SIZE*3); NOTE: NOW INSIDE .bss
	//pmm_alloc_chunk_len(VGA_BUFF2, VGA_SIZE*3); NOTE: NOW INSIDE .bss
	//pmm_alloc_chunk_len(VGA_BUFF3, VGA_SIZE*3); NOTE: NOW INSIDE .bss
	pmm_alloc_chunk_len(
			GDT_BASE - (uint32_t)&__high_half__,
			GDT_D_S * GDT_D_C); // gdt TODO: risky
	//pmm_alloc_chunk_len((uint32_t)(pmm_info->bitmap + pmm_info->size), IDT_SIZE); NOTE: NOW INSIDE .bss
	//pmm_alloc_chunk_len((uint32_t)&_kernel_stack - KSTACK_S, KSTACK_S); NOTE: NOW INSIDE .bss
}

void pmm_putinfo() {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_pmm_info_st;
	uint32_t free_frames = pmm_info->frames_count - pmm_info->alloced;
	vga_putdword("\n  PMM_bitmap_base  : 0x", (uint32_t)&pmm_info->bitmap);
	vga_putsizenm(" size: ", pmm_info->size);
	vga_putnm(" free_frames: ", free_frames);
	vga_putsizenm("x4KB = ", free_frames*PMMF_S);
}

void pmm_init(uint32_t mm_ptr, uint32_t ram_size) {
	volatile struct multiboot_info_st *mbis = (struct multiboot_info_st*)mm_ptr;

	uint8_t *mb_mmap_ptr = (uint8_t*)(mbis->mmap_addr+(uint32_t)&__high_half__);
	uint8_t *mb_mmap_eptr = mb_mmap_ptr + mbis->mmap_length;

	struct multiboot_mmap_st *mb_mmap_st;

	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_pmm_info_st;
	pmm_info->frames_count = ram_size / PMMF_S;
	pmm_info->size = pmm_info->frames_count / 8;
	pmm_info->ram_size = ram_size;

	pmm_info->mm_ptr = mm_ptr;

	pmm_info->alloced = 0;
	for (uint32_t idx = 0; idx < pmm_info->size; idx++) {
		pmm_info->bitmap[ idx ] = 0xff;
		pmm_info->alloced += 8;
	}

	for (; mb_mmap_ptr < mb_mmap_eptr;) {
		mb_mmap_st = (struct multiboot_mmap_st*)mb_mmap_ptr;
		if (mb_mmap_st->type == 1)  
			pmm_free_chunk_len(mb_mmap_st->base_addr_low, mb_mmap_st->length_low);
			
		mb_mmap_ptr += mb_mmap_st->size + 4;
	}

	pmm_alloc_reserved_system();
	pmm_alloc_reserved_kernel();
	pmm_putinfo();
}
