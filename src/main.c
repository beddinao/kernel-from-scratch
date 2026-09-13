#include <kernel.h>

uint32_t phex_b(uint32_t idx, uint8_t c) {
	uint8_t high = ( c >> 4 ) & 0x0f;
	uint8_t low =  c & 0x0f;

	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| (high < 10 ? high + '0' : (high - 10) + 'a');
	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| (low < 10 ? low + '0' : (low - 10) + 'a');

	return idx;
}

uint32_t phex_32(uint32_t idx, uint32_t c) {
	idx = phex_b(idx, c & 0x00FF);
	idx = phex_b(idx, (c & 0xff00) >> 16);
	return idx;
}

uint32_t putBL_name(uint32_t idx, uint32_t p) {
	uint8_t *ptr = (uint8_t*)p;

	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'b';
	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'o';
	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 's';
	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 's';
	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| ':';
	idx++;

	while ( *ptr != '\0' )
		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| *ptr++;

	return idx+1;
}

int kmain(uint32_t magic, uint32_t mm_ptr) {
	clear_screen(VGA_BLUE);

	((uint16_t*)VGA_MEM)[0] = (VGA_BLUE << 4|VGA_WHITE) << 8| '1';
	((uint16_t*)VGA_MEM)[1] = (VGA_BLUE << 4|VGA_WHITE) << 8| '3';
	((uint16_t*)VGA_MEM)[2] = (VGA_BLUE << 4|VGA_WHITE) << 8| '3';
	((uint16_t*)VGA_MEM)[3] = (VGA_BLUE << 4|VGA_WHITE) << 8| '7';

	struct multiboot_info_st *mbd = (struct multiboot_info_st*)mm_ptr;

	((uint16_t*)VGA_MEM)[5] = (VGA_BLUE << 4|VGA_WHITE) << 8| '0';
	((uint16_t*)VGA_MEM)[6] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'x';
	uint32_t idx = phex_b(7, mbd->flags >> 24);
	idx = phex_b(idx, mbd->flags >> 16);
	idx = phex_b(idx, mbd->flags >> 8);
	idx = phex_b(idx, mbd->flags & 0xff);

	if ((mbd->flags >> 9) & 1)
		idx = putBL_name(idx+2, mbd->boot_loader_name);

	if ((mbd->flags >> 6) & 1) {
		idx = phex_b(idx, mbd->mmap_addr >> 8);
		idx = phex_b(idx, mbd->mmap_addr & 0xff);
		idx++;
		idx = phex_b(idx, mbd->mmap_length >> 8);
		idx = phex_b(idx, mbd->mmap_length & 0xff);
	}

	uint8_t *ptr = (uint8_t*)mbd->mmap_addr;
	uint8_t *eptr = ptr + mbd->mmap_length;

	uint64_t ram_size = 0;
	
	uint32_t i = 160;
	for (; ptr <= eptr; i += 80) {
		idx = i;

		uint32_t size = *(ptr+3) << 24 | *(ptr+2) << 16 | *(ptr+1) << 8 | *ptr;

		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| '0';
		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'x';

		idx = phex_b(idx, *(ptr+11)); idx = phex_b(idx, *(ptr+10)); idx = phex_b(idx, *(ptr+9)); idx = phex_b(idx, *(ptr+8));
		idx = phex_b(idx, *(ptr+7)); idx = phex_b(idx, *(ptr+6)); idx = phex_b(idx, *(ptr+5)); idx = phex_b(idx, *(ptr+4));

		idx++;

		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| '0';
		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'x';

		uint32_t length_low = *(ptr+19) << 24 | *(ptr+18) << 16 | *(ptr+17) << 8 | *(ptr+16);
		uint32_t length_high = *(ptr+15) << 24 | *(ptr+14) << 16 | *(ptr+13) << 8 | *(ptr+12);
		uint64_t length = length_low << 32| length_high;
		ram_size += length;


		idx = phex_b(idx, *(ptr+19)); idx = phex_b(idx, *(ptr+18)); idx = phex_b(idx, *(ptr+17)); idx = phex_b(idx, *(ptr+16));
		idx = phex_b(idx, *(ptr+15)); idx = phex_b(idx, *(ptr+14)); idx = phex_b(idx, *(ptr+13)); idx = phex_b(idx, *(ptr+12));

		idx++;

		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| '0';
		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'x';

		idx = phex_b(idx, *(ptr+23)); idx = phex_b(idx, *(ptr+22)); idx = phex_b(idx, *(ptr+21)); idx = phex_b(idx, *(ptr+20));

		ptr += size + 4;
	}
	idx = i + 80;

	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| '0';
	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'x';

	idx = phex_b(idx, (ram_size >> 56) & 0xff); idx = phex_b(idx, ram_size >> 48); idx = phex_b(idx, ram_size >> 40); idx = phex_b(idx, ram_size >> 32);
	idx = phex_b(idx, (ram_size >> 24) & 0xff); idx = phex_b(idx, ram_size >> 16); idx = phex_b(idx, ram_size >> 8); idx = phex_b(idx, ram_size & 0xff);

	(void)magic;
	return 0;
}
