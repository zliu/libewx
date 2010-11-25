#include <cvmx.h>
#include <cvmx-bootmem.h>
#include <cvmx-spinlock.h>
#include "ewx_helper.h"
#include "ewx_block.h"

#define T ewx_block_t

struct ewx_block_s {
	char name[20];              /**< block名称 */
    uint32_t capacity, count;
    uint8_t elem_size;
	void *base_addr;            /**< block基地址指针 */
    uint8_t *used_flag;
    uint32_t freelist;
	cvmx_spinlock_t lock;       /**< 自动锁 */
};

void* ewx_blk(T* block, uint32_t addr)
{
    return ((void *)block->base_addr + (uint64_t)block->elem_size * addr);
}

int ewx_blk_show(T* block, ewx_blk_show_handler_t show)
{
    uint32_t i;
    for (i = 0; i < block->capacity; i++) {
        if (block->used_flag[i] != 0) {
            show(block->base_addr + block->elem_size * i, i);
        }
    }
    return 0;
}

T* ewx_blk_new(uint32_t capacity, uint16_t elem_size, char *name)
{
    uint32_t i;
    T* ptr;
	elem_size = align(elem_size, 128);
	if ((elem_size != 128) && (elem_size != 256)) {
        printf("Err: elem size cannot be aligned");
		return NULL;
	}
    if (name != NULL) {
        ptr = cvmx_bootmem_alloc_named(sizeof(T) + (uint64_t)capacity * elem_size + capacity, elem_size, name);
        if (ptr == NULL) {
            printf("Err: Failed to alloc memory\n");
            return NULL;
        }
        strcpy(ptr->name, name);
    } else {
        ptr = cvmx_bootmem_alloc(sizeof(T) + (uint64_t)capacity * elem_size + capacity, elem_size);
        if (ptr == NULL) {
            printf("Err: Failed to alloc memory\n");
            return NULL;
        }
    }
    ptr->capacity = capacity;
    ptr->elem_size = elem_size;
    ptr->base_addr = (void *)ptr + sizeof(T);
    ptr->used_flag = (void *)ptr->base_addr + (uint64_t)capacity * elem_size;
    for (i = 0; i < ptr->capacity; i++) {
        *(int *)(ptr->base_addr + i * ptr->elem_size) = i - 1;
    }
    ptr->freelist = ptr->capacity - 1;
    memset(ptr->used_flag, 0, capacity);
    cvmx_spinlock_init(&ptr->lock);
    return ptr;
}

int ewx_blk_empty(T* block)
{
    uint32_t i;
    cvmx_spinlock_lock(&block->lock);
    for (i = 0; i < block->capacity; i++) {
        *(int *)(block->base_addr + i * block->elem_size) = i - 1;
    }
    block->freelist = block->capacity - 1;
    memset(block->used_flag, 0, block->capacity);
    cvmx_spinlock_unlock(&block->lock);
    return 0;
}

int ewx_blk_alloc(T* block)
{
    cvmx_spinlock_lock(&block->lock);
    int addr = block->freelist;
    if (addr == -1) {
        cvmx_spinlock_unlock(&block->lock);
        printf("\nERR[--------]: block allocation failed, block is empty\n");
        return -1;
    }
    if (block->used_flag[addr] != 0) {
        cvmx_spinlock_unlock(&block->lock);
        printf("\nERR[--------]: block allocation failed, addr %u is used\n", addr);
        return -1;
    }
    block->used_flag[addr] = 1;
    block->freelist = *(int *)(block->base_addr + addr * block->elem_size);
    cvmx_spinlock_unlock(&block->lock);
    return addr;
}

int ewx_blk_free(T* block, uint32_t addr)
{
    cvmx_spinlock_lock(&block->lock);
    if (block->used_flag[addr] == 0) {
        cvmx_spinlock_unlock(&block->lock);
        printf("Err: block free failed, addr %u is free\n", addr);
        return -1;
    }
    block->used_flag[addr] = 0;
    *(int *)(block->base_addr + addr * block->elem_size) = block->freelist;
    block->freelist = addr;
    cvmx_spinlock_unlock(&block->lock);
    return 0;
}

void ewx_blk_destroy(T* block)
{
}
