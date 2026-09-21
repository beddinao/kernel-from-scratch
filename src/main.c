#include <kernel.h>

void dump_mb_info(uint32_t mm_ptr) {
	volatile struct multiboot_info_st *mbd = (struct multiboot_info_st*)mm_ptr;

	vga_putdword("\nmultiboot flags: 0x", mbd->flags);

	if ((mbd->flags >> 9) & 1) {
		vga_putstr("\nboss: ");
		vga_putstr((uint8_t*)mbd->boot_loader_name);
	}

	if ((mbd->flags >> 6) & 1) {
		vga_putdword("\nmb mmap addr: 0x", mbd->mmap_addr);
		vga_putdword("\nmb mmap length: 0x", mbd->mmap_length);
		vga_putchar('\n');

		uint8_t *ptr = (uint8_t*)mbd->mmap_addr;
		uint8_t *eptr = ptr + mbd->mmap_length;
		struct multiboot_mmap_st* mb_mmap;

		for (; ptr < eptr;) {

			mb_mmap = (struct multiboot_mmap_st*)ptr;

			//_tot_ram_size += mb_mmap->length_high << 32 | mb_mmap->length_low;
			
			vga_putdword("st: 0x", mb_mmap->base_addr_low);
			vga_putdword(" len: 0x", mb_mmap->length_low);
			vga_putdword(" t: 0x", mb_mmap->type);

			ptr += mb_mmap->size + 4;
			vga_putchar('\n');
		}

		//vga_putdword("total ram size: 0x", _tot_ram_size >> 32);
		//vga_putdword("", _tot_ram_size & 0xffffffff);
	}

	if ((mbd->flags >> 3) & 1) {
		vga_putdword("\nloaded boot mods: 0x", mbd->mods_count);
		vga_putdword(" 0x", mbd->mods_addr);
	}
}

int kmain(uint32_t _bootloader_mb_magic, uint32_t mm_ptr) {

	vga_init();

	/*(void)magic;
	vga_putstr("1337");*/

	/*vga_putdword("kernel_start: 0x", (uint32_t)&_kernel_start);
	vga_putdword("\nkernel_end: 0x", (uint32_t)&_kernel_end);
	vga_putdword("\nkernel_stack_top: 0x", (uint32_t)&_kernel_stack);
	vga_putdword("\nkernel_stack_bottom: 0x", (uint32_t)&_kernel_stack - KSTACK_S);
	vga_putdword("\nbootloader_multiboot_magic: 0x", _bootloader_mb_magic);*/

	//dump_mb_info(mm_ptr);

	pmm_init(mm_ptr);
	gdt_init();
	idt_init();

	volatile struct vga_info_st *vga_info = (struct vga_info_st*)VGA_INFO_ST;
	vga_info->attr |= (1 << 4);

	while (1) {
	        asm volatile ("hlt");
	}

	return 0;
}
