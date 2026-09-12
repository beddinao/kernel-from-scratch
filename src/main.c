#include <kernel.h>

uint8_t phex_b(uint8_t idx, uint8_t c) {
	uint8_t high = ( c >> 4 ) & 0x0f;
	uint8_t low =  c & 0x0f;

	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| (high < 10 ? high + '0' : (high - 10) + 'a');
	((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| (low < 10 ? low + '0' : (low - 10) + 'a');

	return idx;
}

uint8_t putBL_name(uint8_t idx, uint32_t p) {
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
	uint8_t idx = phex_b(7, mbd->flags >> 24);
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
	
	for (uint32_t i = 80; ptr < eptr; i += 80) {
		idx = i;

			 *(ptr+2) << 8 | *(ptr+3);
		uint32_t size = *ptr << 24 | *(ptr+1) << 16

		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| '0';
		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'x';

		idx = phex_b(idx, *(ptr+4)); idx = phex_b(idx, *(ptr+5)); idx = phex_b(idx, *(ptr+6)); idx = phex_b(idx, *(ptr+7));
		idx = phex_b(idx, *(ptr+8)); idx = phex_b(idx, *(ptr+9)); idx = phex_b(idx, *(ptr+10)); idx = phex_b(idx, *(ptr+11));

		idx++;

		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| '0';
		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'x';

		idx = phex_b(idx, *(ptr+12)); idx = phex_b(idx, *(ptr+13)); idx = phex_b(idx, *(ptr+14)); idx = phex_b(idx, *(ptr+15));
		idx = phex_b(idx, *(ptr+16)); idx = phex_b(idx, *(ptr+17)); idx = phex_b(idx, *(ptr+18)); idx = phex_b(idx, *(ptr+19));

		idx++;

		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| '0';
		((uint16_t*)VGA_MEM)[idx++] = (VGA_BLUE << 4|VGA_WHITE) << 8| 'x';

		idx = phex_b(idx, *(ptr+20)); idx = phex_b(idx, *(ptr+21)); idx = phex_b(idx, *(ptr+22)); idx = phex_b(idx, *(ptr+23));

		ptr += size + 4;
	}

	(void)magic;
	return 0;
}
