#include <kernel.h>

void kmemset(volatile uint8_t *buff, uint32_t size) {
        for (uint32_t i = 0; i < size; i++)
	      buff[i] = 0;
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
