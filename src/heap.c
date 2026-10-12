#include <kernel.h>

void *kmalloc(uint32_t size) {

	uint32_t chunk_size = 4, indx = 0;
	for (; chunk_size < size && chunk_size <= 2048; chunk_size <<= 1, indx++)
	{}

	heap_node *bucket = (heap_node*)((uint8_t*)&_heap_info_st + indx*sizeof(heap_node));

	if (!bucket->esp) {
		struct heap_info_st *heap_info = (struct heap_info_st*)&_heap_info_st;
		struct vmm_pde *pd = (struct vmm_pde*)((uint32_t)&_boot_page_directory_start + (uint32_t)&__high_half__);
		heap_header *last_page;
		heap_header *pseudo_hdr;
		uint32_t cur_chunk;
		uint32_t new_page = heap_info->heap_brk;

		// TODO: check vpage cache before requesting new ones

		if (vmm_map_page(pd, heap_info->heap_brk, 0b11 << 8 | 0b11) == PMM_FAILURE)
			return 0;

		heap_info->heap_brk += _04KB;

		for (last_page = (heap_header*)bucket->page;
				last_page->next_page != 0;
				last_page = (heap_header*)last_page->next_page)
		{}
		last_page->next_page = (heap_header*)new_page;

		pseudo_hdr = (heap_header*)new_page;
		pseudo_hdr->next_page = 0;
		pseudo_hdr->head = bucket;
		pseudo_hdr->prev_page = last_page;
		pseudo_hdr->used_count = 0;
		pseudo_hdr->depth = last_page->depth+1;

		for (cur_chunk = sizeof(struct heap_header);
				cur_chunk < _04KB - chunk_size && cur_chunk + chunk_size*2 < _04KB;
				cur_chunk += chunk_size)
			*((uint32_t*)(new_page + cur_chunk)) = (new_page + cur_chunk + chunk_size);

		*((uint32_t*)(new_page + cur_chunk)) = 0;
		bucket->esp = new_page + sizeof(struct heap_header);
	}

	uint32_t chunk = bucket->esp;
	bucket->esp = *((uint32_t*)bucket->esp);

	/* TODO: unccomment when vpage cache is done.
	heap_header *header = (heap_header*)(chunk & 0b11111111111111111111000000000000);
	header->used_count += 1;
	*/

	return (void*)chunk;
}

void kfree(void *addr) {
	heap_header *header = (heap_header*)((uint32_t)addr & 0xfffff000);
	heap_node *bucket = header->head;

	/* TODO: uncomment when caching is done.
	if (!header->used_count)
		return;
	*/

	*((uint32_t*)addr) = bucket->esp;
	bucket->esp = (uint32_t)addr;


	/* TODO: transfer the empty vpage to the cache, points the esp to the top free chunk in prev page
	header->used_count -= 1;
	if (!header->used_count && header->depth) {
		if (header->prev_page->used_count >= (_04KB - sizeof(struct heap_header)) / bucket->size)
			return;
		*((uint32_t*)addr & 0b11111111111111111111000000000000) = bucket->vpage_cache;
		bucket->vpage_cache = (uint32_t)addr & 0b11111111111111111111000000000000;

		TODO: actuall free vpage when cache reaches the HEAP_MAX_VPAGE_CACHE limit 
	}
	*/
}

uint32_t ksize(void *addr) {
	heap_header *header = (heap_header*)((uint32_t)addr & 0xfffff000);
	return header->head->size;
}

uint8_t heap_init() {
	struct heap_info_st *heap_info = (struct heap_info_st*)&_heap_info_st;
	struct vmm_pde *pd = (struct vmm_pde*)((uint32_t)&_boot_page_directory_start + (uint32_t)&__high_half__);
	heap_header *pseudo_hdr;
	heap_node *bucket;

	uint32_t chunk_size = 4;
	uint32_t cur_chunk;

	heap_info->heap_brk = HEAP_START;
	heap_info->vpage_cache = 0;

	for (uint8_t i = 0; i < 10; i++, chunk_size *= 2) {
		bucket = (heap_node*)((uint8_t*)heap_info + (i * sizeof(heap_node)));
		bucket->page = heap_info->heap_brk;

		if (vmm_map_page(pd, bucket->page, 0b11 << 8 | 0b11) == PMM_FAILURE)
			return 0;

		pseudo_hdr = (heap_header*)bucket->page;
		pseudo_hdr->head = bucket;
		pseudo_hdr->next_page = 0;
		pseudo_hdr->prev_page = 0;
		pseudo_hdr->used_count = 0;
		pseudo_hdr->depth = 0;

		bucket->size = chunk_size;
		bucket->esp = bucket->page + sizeof(struct heap_header);

		for (cur_chunk = sizeof(struct heap_header);
				cur_chunk < _04KB - chunk_size && cur_chunk+chunk_size*2 < _04KB;
				cur_chunk += chunk_size)
			*((uint32_t*)((uint8_t*)bucket->page + cur_chunk)) = (uint32_t)((uint8_t*)bucket->page + cur_chunk+chunk_size);

		*((uint32_t*)(bucket->page + cur_chunk)) = 0;

		//vga_putdword("\n pseudo_hdr: ", (uint32_t)pseudo_hdr);
		//vga_putnm(" size: ", bucket->size);
		heap_info->heap_brk += _04KB;
	}

	return 1;
}
