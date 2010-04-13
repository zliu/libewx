#ifndef __EWX_MEM_H__
#define __EWX_MEM_H__
#include <cvmx.h>
#include <cvmx-malloc.h>

/** @addtogroup memory_function 内存操作函数 */
//@{
/**
 * 初始化arena，为arena增加内存
 * 
 * @param arena_list area_list的指针，在函数入口前必须分配好指针的空间
 * @param name  名称
 * @param size  大小，单位为字节
 * 
 * @return 0，成功；函数挂死，失败;
 */
int32_t ewx_arena_mem_init(cvmx_arena_list_t *arena_list, uint8_t *name, uint32_t size);

/** 
 * 初始化pool
 * 
 * @param str pool的名称 
 * @param pool pool的id，一般采用config/cvmx-config.h中的宏定义
 * @param block_size 每个block的大小
 * @param block_count block的个数
 * 
 * @return 0，成功；否则，初始化失败；
 */
int32_t ewx_pool_init(uint8_t *str, uint32_t pool, uint32_t block_size, uint32_t block_count);

/** 
 * pool容量调整函数，目前只能调大，不能调小
 * 
 * @param pool pool_num
 * @param block_size pool的block大小
 * @param new_size pool新的大小
 * 
 * @return 0，成功；否则，调整失败；
 */
int32_t ewx_pool_resize(uint32_t pool, uint32_t block_size, uint32_t new_size);

/** 
 * 初始化zone
 * 
 * @param zone_name zone的名称
 * @param block_size zone的block大小
 * @param zone_size zone空间的总大小
 * 
 * @return != NULL, 指向zone的指针；否则，初始化失败；
 */
cvmx_zone_t ewx_zone_init(uint8_t *zone_name, uint32_t block_size, uint32_t zone_size);
//@}
#endif
