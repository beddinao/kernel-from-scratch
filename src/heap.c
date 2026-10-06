#include <kernel.h>

uint32_t kmalloc(uint32_t size) {
	struct heap_info_st *heap_info = (struct heap_info_st*)&_heap_info_st;
	heap_node *node = heap_info->head;
	uint32_t offset = (uint32_t)node; 

	for (;;) {
		offset += node->size + sizeof(heap_node);

		if (node->state == HFREE && node->size >= size) {
			node->state = HUSED;
			node->size = size;
			break;
		}

		/*if (node->state == HUSED && node->size >= (size + sizeof(heap_node))) {
			node->size:
		}*/

		if (!node->next) {
			vmm_map_page(heap_info->pd, offset, 0b11 << 8 | 0b11);
			heap_node *new_node = (heap_node*)offset;
			kmemzero((uint8_t*)new_node, sizeof(heap_node));

			node->next = new_node;
			new_node->prev = node;

			new_node->size = _04KB - sizeof(heap_node);
			new_node->state = HUSED;
			new_node->magic = 0xfa;

			new_node->data = (uint32_t)new_node + sizeof(heap_node);

			node = new_node;
			break;
		}
		node = node->next;
	}

	return node->data;
}

void kfree(uint32_t addr) {
	heap_node *node = addr - sizeof(heap_node);

	if (node->magic == 0xfa && node->state == HUSED)
		node->state = HFREE;
}

void heap_init() {
	struct heap_info_st *heap_info = (struct heap_info_st*)&_heap_info_st;
	heap_info->pd = (struct vmm_pde*)((uint32_t)&_boot_page_directory_start + (uint32_t)&__high_half__);

	vmm_map_page(heap_info->pd, HEAP_START, 0b11 << 8 | 0b11);
	heap_node *head = (heap_node*)HEAP_START;

	kmemzero((uint8_t*)head, sizeof(heap_node));

	head->magic = 0xfa;
	head->state = HFREE;
	head->size = _04KB - sizeof(heap_node);
	head->data = (uint32_t)head + sizeof(heap_node);
	
	heap_info->head = head;

	uint8_t *al = (uint8_t*)kmalloc(10);

	kmemzero(al, 11);
}
