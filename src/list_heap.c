#include <kernel.h>

uint32_t klmalloc(uint32_t size) {
	struct lheap_info_st *lheap_info = (struct lheap_info_st*)&_lheap_info_st;
	lheap_node *node = lheap_info->head;
	uint32_t offset = (uint32_t)node; 

	for (;;) {
		offset += node->bsize + sizeof(lheap_node);

		if (node->state == HFREE && node->bsize >= size) {
			node->state = HUSED;
			node->vsize = size;
			break;
		}

		if (node->state == HUSED && node->bsize >= (size + sizeof(lheap_node))) {
			lheap_node *new_node = (lheap_node*)((uint32_t)node->data + node->vsize);
			kmemzero((uint8_t*)new_node, sizeof(lheap_node));

			node->next = new_node;
			new_node->prev = node;

			new_node->bsize = node->bsize - size - sizeof(lheap_node);
			new_node->vsize = size;
			new_node->state = HUSED;
			new_node->magic = 0xfa;

			new_node->data = (uint32_t)new_node + sizeof(lheap_node);
			node = new_node;
			break;
		}

		if (!node->next) {
			vmm_map_page(lheap_info->pd, offset, 0b11 << 8 | 0b11);
			lheap_node *new_node = (lheap_node*)offset;
			kmemzero((uint8_t*)new_node, sizeof(lheap_node));

			node->next = new_node;
			new_node->prev = node;

			new_node->bsize = _04KB - sizeof(lheap_node);
			new_node->vsize = size;
			new_node->state = HUSED;
			new_node->magic = 0xfa;

			new_node->data = (uint32_t)new_node + sizeof(lheap_node);

			node = new_node;
			break;
		}
		node = node->next;
	}

	return node->data;
}

void klfree(uint32_t addr) {
	lheap_node *node = (lheap_node*)(addr - sizeof(lheap_node));

	if (node->magic == 0xfa && node->state == HUSED)
		node->state = HFREE;
}

void lheap_init() {
	struct lheap_info_st *lheap_info = (struct lheap_info_st*)&_lheap_info_st;
	lheap_info->pd = (struct vmm_pde*)((uint32_t)&_boot_page_directory_start + (uint32_t)&__high_half__);

	vmm_map_page(lheap_info->pd, HEAP_START, 0b11 << 8 | 0b11);
	lheap_node *head = (lheap_node*)HEAP_START;

	kmemzero((uint8_t*)head, sizeof(lheap_node));

	head->vsize = 0;
	head->magic = 0xfa;
	head->state = HFREE;
	head->bsize = _04KB - sizeof(lheap_node);
	head->data = (uint32_t)head + sizeof(lheap_node);
	
	lheap_info->head = head;

	vga_putdword("\n  free-list Heap   : 0x", (uint32_t)&_lheap_info_st);
	vga_putdword(" mem-start: 0x", HEAP_START);

	/*uint8_t *al = (uint8_t*)klmalloc(10);
	kmemzero(al, 11);*/
}
