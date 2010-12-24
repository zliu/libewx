#ifndef __EWX_BLOCK_H__
#define __EWX_BLOCK_H__

#define T ewx_block_t
typedef struct ewx_block_s ewx_block_t;


typedef int32_t (*ewx_blk_show_handler_t)(void *this, uint32_t addr);

extern void* ewx_blk(T* block, uint32_t addr);
extern T* ewx_blk_new(uint32_t capacity, uint16_t elem_size, char* name);
//extern int ewx_blk_empty(T* block);
extern int ewx_blk_alloc(T* block);
extern int ewx_blk_free(T* block, uint32_t addr);
extern void ewx_blk_destroy(T* block);
//extern int ewx_blk_show(T* block, ewx_blk_show_handler_t show);
extern int ewx_blk_is_free(void *ptr);

#endif
