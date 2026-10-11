#include <kernel.h>

void *kmalloc(uint32_t size) {

	uint32_t rsize = 4, indx = 0;
	for (; rsize < size && rsize <= 2048; rsize <<= 1, indx++){}

	vga_putnm("\nalloc: rsize ", rsize);
	vga_putnm(" indx ", indx);
	heap_node *bucket = (heap_node*)((uint8_t*)&_heap_info_st + indx*sizeof(heap_node));

	uint32_t chunk = bucket->esp;
	bucket->esp = *((uint32_t*)bucket->esp);

	if (!bucket->esp) {
		vga_putstr("\nkmalloc: new page alloced");
		struct heap_info_st *heap_info = (struct heap_info_st*)&_heap_info_st;
		struct vmm_pde *pd = (struct vmm_pde*)((uint32_t)&_boot_page_directory_start + (uint32_t)&__high_half__);

		uint32_t new_page = vmm_map_page(pd, heap_info->current_page, 0b11 << 8 | 0b11);
		if (new_page == PMM_FAILURE)
			return 0;
		heap_info->current_page += _04KB;
		
		struct heap_header *pseudo_hdr = (struct heap_header*)new_page;
		pseudo_hdr->prev_page = bucket->page;
		pseudo_hdr->next_page = 0;
		pseudo_hdr->head = bucket;

		pseudo_hdr = (struct heap_header*)bucket->page;
		pseudo_hdr->next_page = new_page;

		for (uint32_t ch = sizeof(struct heap_header); ch < _04KB-rsize; ch += rsize)
			*((uint32_t*)(new_page + ch)) = (new_page + ch + rsize);

		//bucket->esp = new_page + sizeof(struct heap_header);
		*((uint32_t*)bucket->esp) = new_page + sizeof(struct heap_header);
	}

	return (void*)chunk;

}

void kfree(void *addr) {
	struct heap_header *header = (struct heap_header*)((uint32_t)addr & 0b11111111111111111111000000000000);
	heap_node *bucket = header->head;

	//uint32_t offset = (uint32_t)addr & 0b111111111111;
	*((uint32_t*)addr) = bucket->esp;
	bucket->esp = (uint32_t)addr;
}

uint32_t ksize(uint32_t addr) {
	struct heap_header *header = (struct heap_header*)(addr & 0b11111111111111111111000000000000);
	return header->head->size;
}

void heap_init() {
	struct heap_info_st *heap_info = (struct heap_info_st*)&_heap_info_st;
	struct vmm_pde *pd = (struct vmm_pde*)((uint32_t)&_boot_page_directory_start + (uint32_t)&__high_half__);
	heap_node *bucket;
	struct heap_header *pseudo_hdr;

	heap_info->current_page = HEAP_START;
	uint32_t j = 4;

	for (uint8_t i = 0; i < 10; i++, j *= 2) {
		bucket = (heap_node*)((uint8_t*)heap_info + (i * sizeof(heap_node)));
		bucket->page = heap_info->current_page;

		vmm_map_page(pd, bucket->page, 0b11 << 8 | 0b11);

		pseudo_hdr = (struct heap_header*)bucket->page;
		pseudo_hdr->head = bucket;
		pseudo_hdr->next_page = 0;
		pseudo_hdr->prev_page = 0;

		bucket->size = j;
		bucket->esp = bucket->page + sizeof(struct heap_header);

		for (uint32_t ch = sizeof(pseudo_hdr); ch < _04KB - j; ch += j)
			*((uint32_t*)((uint8_t*)bucket->page + ch)) = ((uint8_t*)bucket->page + ch+j);

		vga_putdword("\n pseudo_hdr: ", (uint32_t)pseudo_hdr);
		vga_putnm(" size: ", bucket->size);
		heap_info->current_page += _04KB;
	}

	vga_putdword("\n sizeof(struct heap_header): ", sizeof(struct heap_header));

	uint32_t *new_space = kmalloc(4);
	*new_space = 0;

	vga_putdword("\nnew: 0x", (uint32_t)new_space);
	vga_putdword(": ", ksize((uint32_t)new_space));
	
	//kfree(new_space);

	new_space = kmalloc(2);

	vga_putdword("\nnew: 0x", (uint32_t)new_space);
	vga_putdword(": ", ksize((uint32_t)new_space));

}
