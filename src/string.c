#include <kernel.h>

void kmemzero(volatile uint8_t *buff, uint32_t size) {
        for (uint32_t i = 0; i < size; i++)
		buff[i] = 0;
}

void kmemset(volatile uint8_t *buff, uint8_t ch, uint32_t size) {
	for (uint32_t i=0; i < size; i++)
		buff[i] = ch;
}

void kmemsetw(volatile uint16_t *buff, uint16_t wd, uint32_t size) {
	for (uint32_t i=0; i< size; i++)
		buff[i] = wd;
}

void kmemsetdw(volatile uint32_t *buff, uint32_t dwd, uint32_t size) {
	for (uint32_t i=0; i < size; i++)
		buff[i] = dwd;
}

uint8_t kstrcmp(uint8_t *str1, uint8_t *str2) {
        for (; *str1 != 0 && *str2 != 0 && *str1 == *str2; str1++, str2++);
        return *str1 - *str2;
}

uint16_t kstrlen(uint8_t *str) {
        uint16_t len = 0;
        for (; *str != 0; str++)
	      len++;
        return len;
}

uint32_t ktonum(uint8_t *str) {
	uint32_t nm = 0;
	for (; *str >= '0' && *str <= '9'; str++) 
		nm = nm*10 + (*str-'0');
	return nm;
}
