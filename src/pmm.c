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
	for (; base_addr < enptr; base_addr++)
		pmm_free(base_addr);
}

void pmm_dump_mmap() {
	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_kernel_end;

	vga_putchar('\n');

	for (uint8_t idx = 0; idx < 10; idx++) {
		for (uint8_t b = 0; b < 8; b++) {
			vga_putchar( ( pmm_info->bitmap[ idx ] & (1 << b) ) ? '1' : '0');
		}
	}
	vga_putchar('\n');
}

void pmm_init(uint32_t mm_ptr) {
	struct multiboot_info_st *mbis = (struct multiboot_info_st*)mm_ptr;
	uint8_t *mb_mmap_ptr = (uint8_t*)mbis->mmap_addr;
	uint8_t *mb_mmap_eptr = mb_mmap_ptr + mbis->mmap_length;
	struct multiboot_mmap_st *mb_mmap_st;

	struct pmm_info_st *pmm_info = (struct pmm_info_st*)&_kernel_end;
	pmm_info->frames_count = 0xffffffff / PMMF_S;
	pmm_info->size = pmm_info->frames_count / 8;

	vga_putstr("\n\n");
	vga_putdword("pmm_base_addr: 0x", (uint32_t)&pmm_info->bitmap);
	vga_putdword("\npmm_frames_count: 0x", pmm_info->frames_count);
	vga_putdword("\npmm_size: 0x", pmm_info->size);


	for (uint8_t idx = 0; idx < pmm_info->size; idx++)
		pmm_info->bitmap[ idx ] = 0xff;

	/*
	for (; mb_mmap_ptr < mb_mmap_eptr;) {
		mb_mmap_st = (struct multiboot_mmap_st*)mb_mmap_ptr;
		if (mb_mmap_st->type == 1) 
			pmm_free_chunk(mb_mmap_st->base_addr_low, mb_mmap_st->length_low);
			
		mb_mmap_ptr += mb_mmap_st->size + 4;
	}*/

	pmm_dump_mmap();
}
