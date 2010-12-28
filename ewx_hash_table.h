#ifndef __EWX_HT_H__
#define __EWX_HT_H__

#include "ewx.h"
#include <cvmx-spinlock.h>

#define LOCK_SUFFIX "_spinlock"
#define HASH_TABLE_ADDR_SHIFT 7 /*128 bytes对齐*/

/// ewx hash_table 头部数据结构
typedef struct ewx_hash_table {
	char name[20];				/**< hash表名称 */
	uint32_t bucket_num;		/**< 桶的个数 */
	uint16_t bucket_size;	    /**< 每个桶的大小 */
	uint8_t item_size;	        /**< 每个桶item的大小 */
	uint8_t item_num; 	        /**< 每个桶有多少个item，根据上面的size计算出来 */
	void *base_ptr;				/**< hash表基地址指针 */
	cvmx_spinlock_t *lock;		/**< 自动锁 */
} ewx_hash_table_t;

/// ewx hash_table bucket头部数据结构
typedef struct ewx_bucket_hd {
	uint32_t next_offset;		/**< 下一个桶的地址 */
	uint8_t valid_count;		/**< 当前桶里的表项个数*/
	uint8_t reserved[3];		/**< 保留 */
} ewx_bucket_hd_t;
// 结束 hash表内部使用数据结构

/** @defgroup hash_table_op hash表操作 */
//@{
//@}

/** @defgroup hash_table_callback hash表回调函数
 *  @ingroup hash_table_op
*/
//@{
// 开始 用户需要实现的回调函数
/**
 * 节点匹配回调函数
 *
 * @param this 需要比较的节点数据
 * @param user_data 用户的数据
 * @param table_item hash表中的节点
 * @return 返回0，匹配；否则，不匹配
 */
typedef int32_t (*ewx_hash_table_compare_handle_t)(void *this, void *user_data, void *table_item);


/**
 * hash表插入节点回调函数，常用在hash插入时，视情况而定插入/更新节点的场合(比如有的场合下，搜索是否有一个特征匹配的节点，如
 * 果有就修改该节点，否则，便插入一个新节点)
 *
 * @param this 当前节点
 * @param user_data 用户的数据
 * @param table_item hash表中的节点
 * @return >= 0 表示通过该回调已经成功插入;
 *  return < 0 表示插入失败，不会插入新节点，直接返回;
 */
typedef int32_t (*ewx_hash_table_insert_handle_t)(void *this, void *user_data, void *table_item);

/**
 * hash表更新节点回调函数，常用在hash插入时，视情况而定插入/更新节点的场合(比如有的场合下，搜索是否有一个特征匹配的节点，如
 * 果有就修改该节点，否则，便插入一个新节点)
 *
 * @param this 当前节点
 * @param user_data 用户的数据
 * @param table_item hash表中的节点
 * @return > 0 表示通过该回调已经成功更新，不需要再插入新节点；
 *  return == 0 表示插入函数成功更新，但仍需要插入新节点;
 *  return < 0 表示更新失败，不会插入新节点，直接返回；
 */
typedef int32_t (*ewx_hash_table_update_handle_t)(void *this, void *user_data, void *table_item);

/**
 * bucket分配回调函数，当一个bucket满的时候，需要通过该回调分配空间
 *
 * @param size 一个bucket所占用的字节数
 * @return != NULL,返回分配好的指针；== NULL,表示返回失败;
 */
typedef void *(*ewx_hash_table_bucket_alloc_handle_t)(int size);

/**
 * hash表节点删除回调函数，常用在hash插入时，视情况而定删除节点的场合(比如有的场合下，搜索是否有一个特征匹配的节点，如果有就修改该节点，
 * 否则，便删除一个新节点)
 *
 * @param this 当前节点
 * @param user_data 用户的数据
 * @param table_item hash表中的节点
 * @return > 0 表示通过该回调已经成功删除，不需要再删除真正的节点；
 *  return == 0 表示插入函数成功删除，但需要把真正的节点都删除;
 *  return < 0 表示删除失败，直接返回；
 */
typedef int (*ewx_hash_table_remove_handle_t)(void *this, void *user_data, void *table_item);

/**
 * bucket释放回调函数，当一个bucket所有节点都被释放时，需要通过该函数释放该bucket，如果这个函数为NULL，bucket不能释放，会导致内存泄露
 *
 * @param bucket_p 当前bucket
 *
 * @return 无
 */
typedef void (*ewx_hash_table_bucket_free_handle_t)(void *bucket_p);

/**
 * hash表显示回调函数
 *
 * @param table_item hash表中的节点
 *
 * @return 无
 */
typedef void (*ewx_hash_table_show_handle_t)(void *table_item, uint32_t hash);

//@} hash表回调函数
// 结束 用户需要实现的回调函数


/** @defgroup hash_table_function hash表操作函数
 *  @ingroup hash_table_op
 */
//@{

/* hash表初始化函数
 *
 * @param name hash表名称
 * @param bucket_num 桶的数目
 * @param bucket_size 每个桶的大小
 * @param item_size 每个节点的大小
 * @param auto_lock 是否自动加锁
 *
 * @return != NULL，返回hash表的指针；否则，表示初始化失败；
 */
ewx_hash_table_t *ewx_hash_table_init(char *name, int bucket_num, int bucket_size, int item_size, int auto_lock);

/**
 * hash表节点搜索函数
 *
 * @param hash_table_p hash表指针
 * @param hash 要搜索节点的hash值
 * @param compare 节点匹配回调函数
 * @param this 当前搜索的节点
 * @param user_data 用户数据，在某些情况下，除了节点数据外，可能还需要额外的用户数据用来进行匹配
 * @return != NULL，返回匹配的节点指针，否则，不匹配
 */
void *ewx_hash_table_search(ewx_hash_table_t *hash_table_p, uint32_t hash, ewx_hash_table_compare_handle_t compare,
							void *this, void *user_data);

/**
 * hash表节点插入函数
 *
 * @param hash_table_p hash表指针
 * @param hash 要搜索节点的hash值
 * @param free_pos_p 如果已经知道要插入的位置，设置该值，否则为NULL；
 * @param compare 可选的节点匹配回调函数
 * @param this 可选的当前搜索的节点
 * @param user_data 可选的用户数据，在某些情况下，除了节点数据外，可能还需要额外的用户数据用来进行匹配
 * @param update 可选的节点更新回调函数
 * @param bucket_alloc 可选的bucket分配回调函数，如果该回调为NULL，那么bucket将不能往后链
 *
 * @return 0，插入成功；否则，插入失败
 */
int32_t ewx_hash_table_insert(ewx_hash_table_t *hash_table_p, uint32_t hash, void *free_pos_p, ewx_hash_table_compare_handle_t compare,
							  void *this, void *user_data, ewx_hash_table_update_handle_t update,
							  ewx_hash_table_bucket_alloc_handle_t bucket_alloc);

int32_t ewx_hash_table_insert2(ewx_hash_table_t *hash_table_p, uint32_t hash, void *free_pos_p,
                               ewx_hash_table_compare_handle_t compare,
							  void *this, void *user_data, ewx_hash_table_insert_handle_t update,
                              ewx_hash_table_insert_handle_t insert,
							  ewx_hash_table_bucket_alloc_handle_t bucket_alloc);


/**
 * hash表节点删除函数
 *
 * @param hash_table_p hash表指针
 * @param hash 要搜索节点的hash值
 * @param compare 可选的节点匹配回调函数
 * @param this 当前搜索的节点
 * @param user_data 可选的用户数据，在某些情况下，除了节点数据外，可能还需要额外的用户数据用来进行匹配
 * @param remove 可选的节点删除回调函数
 * @param bucket_free 可选的bucket释放分配函数，如果该回调为NULL，在insert的bucket_alloc回调不为空时，可能会导致内存泄露
 *
 * @return 0，插入成功；否则，插入失败；
 */
int32_t ewx_hash_table_remove(ewx_hash_table_t *hash_table_p, uint32_t hash, ewx_hash_table_compare_handle_t compare, void *this,
							  void *user_data, ewx_hash_table_remove_handle_t remove, ewx_hash_table_bucket_free_handle_t bucket_free);

/**
 * hash表显示函数
 *
 * @param hash_table_p hash表指针
 * @param show 节点显示回调函数
 */
void ewx_hash_table_show(ewx_hash_table_t *hash_table_p, ewx_hash_table_show_handle_t show);

// @} hash表操作函数


#endif
