#include <kernel.h>

void pmm_alloc_frame(uint32_t addr) {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_kernel_end;
	uint32_t frame_idx = (addr / PMMF_S);
	pmm_info->bitmap[ frame_idx / 8 ] |= 1 << (frame_idx % 8);
	// 11011[0]11 | 00000[1]00
}

void pmm_free(uint32_t addr) {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_kernel_end;
	uint32_t frame_idx = (addr / PMMF_S);
	pmm_info->bitmap[ frame_idx / 8 ] &= ~(1 << (frame_idx % 8));
	// 01000[1]00 & 11111[0]11 
	//vga_putdword("\nfree: 0x", addr);
	//vga_putchar('\n');
}

uint32_t pmm_alloc() {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_kernel_end;
	uint8_t frame_bit_idx;
	uint8_t frame_byte_idx;

	for (uint32_t frame_idx = 0; frame_idx < pmm_info->frames_count; frame_idx++) {

		frame_bit_idx = frame_idx % 8;
		frame_byte_idx = frame_idx / 8;
		
		if (! ((pmm_info->bitmap[ frame_byte_idx ] >> frame_bit_idx) & 1) ) {
			pmm_info->bitmap[ frame_byte_idx ] |= 1 << frame_bit_idx;
			return frame_idx * PMMF_S;
		}

	}
	return 0;
}

void pmm_free_chunk(uint32_t base_addr, uint32_t length) {
	uint32_t enptr = base_addr + length;
	for (; base_addr < enptr; base_addr += PMMF_S)
		pmm_free(base_addr);
}

void pmm_alloc_chunk_len(uint32_t base_addr, uint32_t length) {
	vga_putdword("\nalloc_chunk: 0x", base_addr);
	vga_putdword(" len: 0x", length);
	uint32_t enptr = base_addr + length;
	for (; base_addr < enptr; base_addr += PMMF_S)
		pmm_alloc(base_addr);
}

void pmm_alloc_chunk_eptr(uint32_t base_addr, uint32_t eptr) {
	vga_putdword("\nalloc_chunk: 0x", base_addr);
	vga_putdword(" eptr: 0x", eptr);
	for (; base_addr < eptr; base_addr += PMMF_S)
		pmm_alloc(base_addr);
}

void pmm_dump_mmap(uint32_t sidx, uint32_t eidx) {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_kernel_end;

	vga_putdword("\npmm_mmap frames dump from 0x", sidx);
	vga_putdword(" to 0x", eidx);
	vga_putstr(": ");
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
}

void pmm_alloc_reserved_kernel() {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_kernel_end;
	pmm_alloc_chunk_eptr((uint32_t)&_kernel_start, (uint32_t)&_kernel_end); // kernel itself
	pmm_alloc_chunk_len((uint32_t)&_kernel_end, pmm_info->size); // pmm bitmap
	pmm_alloc_chunk_len(VGA_INFO_ST, sizeof(struct vga_info_st)); // vga info
}

void pmm_init(uint32_t mm_ptr) {
	struct multiboot_info_st *mbis = (struct multiboot_info_st*)mm_ptr;
	uint8_t *mb_mmap_ptr = (uint8_t*)mbis->mmap_addr;
	uint8_t *mb_mmap_eptr = mb_mmap_ptr + mbis->mmap_length;
	struct multiboot_mmap_st *mb_mmap_st;

	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_kernel_end;
	pmm_info->frames_count = 0xffffffff / PMMF_S;
	pmm_info->size = pmm_info->frames_count / 8;

	vga_putdword("\npmm_base_addr: 0x", (uint32_t)&pmm_info->bitmap);
	vga_putdword("\npmm_frames_count: 0x", pmm_info->frames_count);
	vga_putdword("\npmm_size: 0x", pmm_info->size);

	for (uint32_t idx = 0; idx < pmm_info->size; idx++)
		pmm_info->bitmap[ idx ] = 0xff;

	for (; mb_mmap_ptr < mb_mmap_eptr;) {
		mb_mmap_st = (struct multiboot_mmap_st*)mb_mmap_ptr;
		if (mb_mmap_st->type == 1) 
			pmm_free_chunk(mb_mmap_st->base_addr_low, mb_mmap_st->length_low);
			
		mb_mmap_ptr += mb_mmap_st->size + 4;
	}

	pmm_alloc_reserved_system();
	pmm_alloc_reserved_kernel();

	/*pmm_free_chunk(PMMF_S, PMMF_S * 2);
	pmm_dump_mmap(0, 5);
	uint32_t fframe = pmm_alloc();
	uint32_t sframe = pmm_alloc();
	vga_putdword("pmm_alloc: 0x", fframe);
	vga_putdword("\npmm_alloc: 0x", sframe);
	pmm_free(fframe);
	vga_putdword("\npmm_alloc: 0x", pmm_alloc());
	pmm_dump_mmap(0, 5);
	*/
}
