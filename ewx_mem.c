#include <cvmx.h>
#include <cvmx-bootmem.h>
#include <cvmx-malloc.h>
#include <cvmx-fpa.h>
#include <assert.h>
#include "ewx_code.h"

int32_t ewx_arena_mem_init(cvmx_arena_list_t *arena_list, uint8_t *arena_name, uint32_t arena_size)
{
	int result;

	void *ptr = cvmx_bootmem_alloc_named(arena_size, 128, (char *)arena_name);
	assert(ptr != NULL);

	result = cvmx_add_arena(arena_list, ptr, arena_size);
	assert(result == 0);

	return 0;
}

int32_t ewx_pool_init(uint8_t *str, uint32_t pool, uint32_t block_size, uint32_t pool_count)
{
	void *memory;
	memory = cvmx_bootmem_alloc((block_size * pool_count), CVMX_CACHE_LINE_SIZE);
	if (memory == NULL) {
		return -EWX_NO_SPACE_ERROR;
	}

	cvmx_fpa_setup_pool(pool, (char *)str, memory, block_size, pool_count);

	return 0;
}

int32_t ewx_pool_resize(uint32_t pool, uint32_t block_size, uint32_t new_size)
{
	uint64_t old_size = cvmx_read_csr(CVMX_FPA_QUEX_AVAILABLE(pool)) * block_size;
	new_size = (uint64_t)new_size * 1024 * 1024;
	void *memory;
	char *ptr;
	int num_block;

	//printf("old_size=%llu, new_size=%llu\n", old_size, new_size);
	if(old_size >= new_size)
		return 0;
	else{
		num_block = (new_size-old_size+block_size - 1) / (block_size);
		memory = cvmx_bootmem_alloc(num_block*block_size, CVMX_CACHE_LINE_SIZE);
		if (memory == NULL) {
			return -EWX_NO_SPACE_ERROR;
		}
		ptr = (char*)memory;
		while (num_block--) {
			cvmx_fpa_free(ptr, pool, 0);
        	ptr += block_size;
		}
	}
	return 0;
}

/*通用堆栈操作函数*/
cvmx_zone_t ewx_zone_init(uint8_t *zone_name, uint32_t block_size, uint32_t zone_size)
{
	void *ptr;
	cvmx_zone_t zone;

	ptr = cvmx_bootmem_alloc_named(zone_size+128, 128, (char *)zone_name);
	if (ptr == NULL) {
		return NULL;
	}

	zone = cvmx_zone_create_from_addr((char *)zone_name, block_size, zone_size/block_size,
									  ptr, zone_size+128, 0);
	if (zone == NULL) {
		cvmx_bootmem_free_named((char *)zone_name);
		return NULL;
	}

	return zone;
}

int32_t ewx_heap_add_mem(cvmx_arena_list_t *heap, int size)
{
	int result;

	void *ptr = cvmx_bootmem_alloc(size, 8);
	if (ptr == NULL) {
		return -EWX_NO_SPACE_ERROR;
	}

	result = cvmx_add_arena(heap, ptr, size);
	return result;
}


/*下面是一些内存操作接口的实例*/

/*通用堆栈操作函数*/


