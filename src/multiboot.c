#include <kernel.h>

uint32_t inspect_mb_info(uint32_t _magic, uint32_t mm_ptr) {
	volatile struct multiboot_info_st *mbd = (struct multiboot_info_st*)mm_ptr;
	uint32_t _tot_ram_size = 0;

	vga_putchar('\n');
	if (_magic != MB1_MAGIC_NM)
		panic("invalid multiboot protocol!");

	if ((mbd->flags >> 6) & 1) {

		uint8_t *ptr = (uint8_t*)(mbd->mmap_addr+(uint32_t)&__high_half__);
		uint8_t *eptr = ptr + mbd->mmap_length;
		struct multiboot_mmap_st* mb_mmap;

		for (; ptr < eptr;) {

			mb_mmap = (struct multiboot_mmap_st*)ptr;

			_tot_ram_size += mb_mmap->length_low;
			if (mb_mmap->type == 1) {

				vga_putdword("  mem_region_start : 0x", mb_mmap->base_addr_low);
				vga_putdword(" length: 0x", mb_mmap->length_low);
				vga_putsizenm(" ", mb_mmap->length_low);
				vga_putstr(" type: FREE\n");
			}

			ptr += mb_mmap->size + 4;
		}

		vga_putdword("  RAM size         : 0x", _tot_ram_size);
		vga_putsizenm(" ", _tot_ram_size);
		vga_putdword(", __high_half__: 0x", (uint32_t)&__high_half__);
		vga_putsizenm(" ", (uint32_t)&__high_half__);
		if (_tot_ram_size < MIN_RAM)
			panic("insufficant memory at least 10Mb");
	}
	else panic("multiboot memory map is not present as requested!");
	return _tot_ram_size;
}
