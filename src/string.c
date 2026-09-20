#include <kernel.h>



void kmemset(volatile uint8_t *buff, uint32_t size) {
        for (uint32_t i = 0; i < size; i++)
	      buff[i] = 0;
}
