#include <cvmx.h>
#include <cvmx-bootmem.h>
#include <cvmx-spinlock.h>
#include <cvmx-atomic.h>
#include "ewx_helper.h"
#include "ewx_block.h"
#include "ewx_debug.h"

#define T ewx_block_t

struct ewx_block_s {
	char            name[20];               /**< block名称 */
    uint32_t        capacity;               /**< block容量 */
    int32_t         count;                  /**< block已分配使用的数量 */
    uint8_t         elem_size;              /**< 每个block的大小 */
	void *          base_addr;              /**< block基地址指针 */
    int32_t         freelist;               /**< 空闲block链表的头指针, 指针值采用内部index */
	cvmx_spinlock_t lock;                   /**< 空闲block链表的锁 */
    uint8_t *       used;                   /**< 每个block是否已使用的标记 */
};


inline static int * __next_blk(T* block, uint32_t addr)
{
    return (int32_t *)(block->base_addr + (uint64_t)addr * block->elem_size);
}

void* ewx_blk(T* block, uint32_t addr)
{
    return (void *)(block->base_addr + (uint64_t)addr * block->elem_size);
}

T* ewx_blk_new(uint32_t capacity, uint16_t elem_size, char *name)
{
    uint32_t i;
    T* ptr;
    if (name != NULL) {
        ptr = cvmx_bootmem_alloc_named(sizeof(T) + (uint64_t)capacity * elem_size + capacity/*used array*/, 128, name);
        if (ptr == NULL) {
            printd(1, "\nERR[--------]: Failed to alloc memory\n");
            return NULL;
        }
        strcpy(ptr->name, name);
    } else {
        ptr = cvmx_bootmem_alloc(sizeof(T) + (uint64_t)capacity * elem_size + capacity/*used array*/, 128);
        if (ptr == NULL) {
            printd(1, "\nERR[--------]: Failed to alloc memory\n");
            return NULL;
        }
        ptr->name[0] = 0;
    }
    ptr->capacity = capacity;
    ptr->count = 0;
    ptr->elem_size = elem_size;
    ptr->base_addr = (void *)ptr + sizeof(T);
    ptr->used = ptr->base_addr + (uint64_t)capacity * elem_size;
    for (i = 0; i < ptr->capacity; i++) {
        *__next_blk(ptr, i) = i + 1;
        ptr->used[i] = 0;
    }
    *__next_blk(ptr, ptr->capacity - 1) = -1;
    ptr->freelist = 0;
    cvmx_spinlock_init(&ptr->lock);
    return ptr;
}


int ewx_blk_empty(T* block)
{
    uint32_t i;
    cvmx_spinlock_lock(&block->lock);

    for (i = 0; i < block->capacity; i++) {
        *__next_blk(block, i) = i + 1;
        block->used[i] = 0;
    }
    *__next_blk(block, block->capacity) = -1;
    block->freelist = 0;
    cvmx_atomic_set32(&block->count, 0);
    cvmx_spinlock_unlock(&block->lock);
    return 0;
}


int ewx_blk_alloc(T* block)
{
    cvmx_spinlock_lock(&block->lock);
    int addr = block->freelist;
    if (addr < 0) {
        cvmx_spinlock_unlock(&block->lock);
        printd(1, "\nERR[--------]: block allocation failed, block is empty\n");
        return -1;
    }
    if (block->used[addr]) {
        cvmx_spinlock_unlock(&block->lock);
        printd(1, "\nERR[--------]: block allocation failed, addr %u is used\n", addr);
        return -1;
    }
    block->freelist = *__next_blk(block, addr);
    block->used[addr] = 1;
    cvmx_spinlock_unlock(&block->lock);
    cvmx_atomic_add32(&block->count, 1);
    return addr;
}

int ewx_blk_free(T* block, uint32_t addr)
{
    cvmx_spinlock_lock(&block->lock);
    if (!block->used[addr]) {
        cvmx_spinlock_unlock(&block->lock);
        printd(1, "\nERR[--------]: block free failed, addr %u is free\n", addr);
        return -1;
    }
    *__next_blk(block, addr) = block->freelist;
    block->used[addr] = 0;
    block->freelist = addr;
    cvmx_spinlock_unlock(&block->lock);
    cvmx_atomic_add32(&block->count, -1);
    return 0;
}

void ewx_blk_destroy(T* block)
{
    if (block->name[0] == 0) {
        printd(1, "\nError[--------]: block destroy failed, cannot free a unnamed block\n");
    } else {
        cvmx_bootmem_free_named(block->name);
    }
}
