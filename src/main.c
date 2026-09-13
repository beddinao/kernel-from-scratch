#include <kernel.h>

int kmain(uint32_t magic, uint32_t mm_ptr) {

	vga_init();

	(void)magic;
	vga_putstr("1337");

	struct multiboot_info_st *mbd = (struct multiboot_info_st*)mm_ptr;

	vga_putdword("\nmultiboot flags: 0x", mbd->flags);

	if ((mbd->flags >> 9) & 1) {
		vga_putstr("\nboss: ");
		vga_putstr((uint8_t*)mbd->boot_loader_name);
	}

	if ((mbd->flags >> 6) & 1) {
		vga_putdword("\nmb mmap addr: 0x", mbd->mmap_addr);
		vga_putdword("\nmb mmap length: 0x", mbd->mmap_length);
		vga_putchar('\n');
	}

	uint8_t *ptr = (uint8_t*)mbd->mmap_addr;
	uint8_t *eptr = ptr + mbd->mmap_length;
	struct multiboot_mmap_st* mb_mmap;
	uint64_t ram_size = 0;
	
	for (; ptr < eptr;) {

		mb_mmap = (struct multiboot_mmap_st*)ptr;

		ram_size += mb_mmap->length_high << 32 | mb_mmap->length_low;

		vga_putdword("st: 0x", mb_mmap->base_addr_low);
		vga_putdword("", mb_mmap->base_addr_high);
		vga_putdword(" len: 0x", mb_mmap->length_low);
		vga_putdword("", mb_mmap->length_high);
		vga_putdword(" t: 0x", mb_mmap->type);

		ptr += mb_mmap->size + 4;
		vga_putchar('\n');
	}

	vga_putdword("ram_size: 0x", ram_size >> 32);
	vga_putdword("", ram_size & 0xffffffff);

	return 0;
}
