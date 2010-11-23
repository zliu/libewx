#include <cvmx.h>
#include <cvmx-bootmem.h>
#include <cvmx-spinlock.h>
#include "ewx_helper.h"
#include "ewx_block.h"

typedef struct __stack_int_t {
    uint32_t capacity, count;
    uint32_t *base;
} __stk_int_t;

__stk_int_t *__stk_new(uint32_t capacity)
{
    uint32_t i;
    __stk_int_t *ptr = cvmx_bootmem_alloc(sizeof(__stk_int_t) + (uint64_t)capacity * sizeof(uint32_t), 128);
    if (ptr == NULL) {
        printf("Err: Failed to alloc memory.\n");
        return NULL;
    }
    ptr->capacity = capacity;
    ptr->base = (uint32_t *)(ptr + 1);
    for (i = 0; i < capacity; i++) {
        *(ptr->base + i) = capacity - i;
    }
    ptr->count = capacity;
    return ptr;
}

int __stk_push(__stk_int_t *stk, uint32_t value)
{
    if (stk->capacity < stk->count + 1) {
        printf("Err: Stack is full, failed to push.");
        return -1;
    }
    stk->base[stk->count++] = value;
    return 0;
}

int __stk_pop(__stk_int_t *stk)
{
    if (stk->count == 0) {
        printf("Err: Stack if empty, failed to pop.");
        return -1;
    }
    return stk->base[stk->count--];
}

void __stk_empty(__stk_int_t *stk)
{
    stk->count = 0;
}

void __stk_destroy(__stk_int_t *stk)
{
}

#define T ewx_block_t

struct ewx_block_s {
	char name[20];              /**< block名称 */
    uint32_t capacity, count;
    uint8_t elem_size;
	void *base_addr;            /**< block基地址指针 */
    uint8_t *used_flag;
    __stk_int_t *freelist;
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
    memset(ptr->used_flag, 0, capacity);
    ptr->freelist = __stk_new(capacity);
    cvmx_spinlock_init(&ptr->lock);
    return ptr;
}

void ewx_blk_empty(T* block)
{
    __stk_empty(block->freelist);
    memset(block->used_flag, 0, block->capacity);
}

int ewx_blk_alloc(T* block)
{
    int addr = __stk_pop(block->freelist);
    if ((addr != -1) && (block->used_flag[addr] == 0)) {
        block->used_flag[addr] = 1;
        return addr;
    } else {
        if (block->used_flag[addr] != 0)
        printf("Err: block alloc failed, addr %u used\n", addr);
        return -1;
    }
}

int ewx_blk_free(T* block, uint32_t addr)
{
    if (block->used_flag[addr] != 0) {
        block->used_flag[addr] = 0;
        return __stk_push(block->freelist, addr);
    } else {
        printf("Err: block free failed, addr %u not used\n", addr);
        return -1;
    }
}

void ewx_blk_destroy(T* block)
{
}
