/**
 * @file   ewx_hash_table.c
 * @author xdzhou <zhou.xiaodong@embedway.com>
 * @date   Wed Mar  3 17:36:48 2010
 * 
 * @brief  这个是hash表的库
 * 
 * 
 */
#include <cvmx.h>
#include <cvmx-spinlock.h>
#include <cvmx-bootmem.h>
#include "ewx_code.h"
#include "ewx_helper.h"
#include "ewx_hash_table.h"

ewx_hash_table_t *ewx_hash_table_init(char *name, int bucket_num, int bucket_size, int item_size, int auto_lock)
{
	int i;

	bucket_size = align(bucket_size, 128);

	if ((bucket_size != 128) && (bucket_size != 256)) {
		return NULL;
	}

	ewx_hash_table_t *ptr = cvmx_bootmem_alloc_named((uint64_t)(bucket_num) * bucket_size + sizeof(ewx_hash_table_t), 128, name);
	if (ptr == NULL) {
		return NULL;
	}
	memset(ptr, 0, (uint64_t)bucket_num * bucket_size + sizeof(ewx_hash_table_t));

	strcpy(ptr->name, name);

	ptr->bucket_num = bucket_num;
	ptr->bucket_size = bucket_size;
	ptr->item_size = item_size;
	ptr->item_num = (bucket_size - sizeof(ewx_bucket_hd_t))/item_size;
	ptr->base_ptr = ptr + sizeof(ewx_hash_table_t);

	if (auto_lock) {
		char lock_name[30];
		strcpy(lock_name, name);
		strcat(lock_name, LOCK_SUFFIX);
		ptr->lock = cvmx_bootmem_alloc_named(sizeof(cvmx_spinlock_t) * (uint64_t)bucket_num, 128, lock_name);
		if (ptr->lock == NULL) {
			/*释放hash表空间，返回NULL*/
			cvmx_bootmem_free_named(name);
			return NULL;
		}
		for (i=0; i<bucket_num; i++) {
			cvmx_spinlock_init(&ptr->lock[i]);
		}
	} else {
		ptr->lock = NULL;
	}
	return ptr;
}

void *ewx_hash_table_search(ewx_hash_table_t *hash_table_p, uint32_t hash, ewx_hash_table_compare_handle_t compare, void *this, 
							void *user_data)
{
	
	ewx_bucket_hd_t *current, *next;
	int i, found = 0;
	void *data;

	if (cvmx_unlikely(hash_table_p->bucket_num <= hash)) {
		return NULL;
	}
	
	if (hash_table_p->lock != NULL) {
		cvmx_spinlock_lock(&hash_table_p->lock[hash]);
	}
	/*开始搜索*/
	current = (ewx_bucket_hd_t *)((void *)hash_table_p->base_ptr + (uint64_t)(hash_table_p->bucket_size) * hash);
	do {
		if (current->next_offset != 0) {
			next = (ewx_bucket_hd_t *)cvmx_phys_to_ptr(((uint64_t)current->next_offset) << 7);
			//CVMX_PREFETCH(next, 0);
		} else {
			next = NULL;
		}
		data = (void *)(current + 1);/*数据开头紧跟着bucket头之后*/
		
		for (i=0; i<current->valid_count; i++) {
			if (compare(this, user_data, data) == 0) {
				found = 1;
				break;
			}
			data += hash_table_p->item_size;
		}
		current = next;
	} while ((next != NULL) && (found == 0)); 
	
	/*搜索结束*/
	if (hash_table_p->lock != NULL) {
		cvmx_spinlock_unlock(&hash_table_p->lock[hash]);
	}
	
	if (found) {
		return data;
	} else {
		return NULL;
	}
}

int32_t ewx_hash_table_insert(ewx_hash_table_t *hash_table_p, uint32_t hash, void *free_pos_p, ewx_hash_table_compare_handle_t compare, 
							  void *this, void *user_data, ewx_hash_table_insert_handle_t insert, 
							  ewx_hash_table_bucket_alloc_handle_t bucket_alloc)
{

	ewx_bucket_hd_t *current, *next, *pre;
	int i, result = 0;
	void *data;

	if (hash_table_p->lock != NULL) {
		cvmx_spinlock_lock(&hash_table_p->lock[hash]);
	}

	current = (ewx_bucket_hd_t *)((void *)hash_table_p->base_ptr + (uint64_t)(hash_table_p->bucket_size) * hash);
	pre = current;
	if (free_pos_p == NULL) {
		do {
			if (current->next_offset != 0) {
				next = (ewx_bucket_hd_t *)cvmx_phys_to_ptr(((uint64_t)current->next_offset) << 7);
				CVMX_PREFETCH(next, 0);
			} else {
				next = NULL;
			}
			data = (void *)(current + 1);/*数据开头紧跟着bucket头之后*/		
			if (compare != NULL) {
				for (i=0; i<current->valid_count; i++) {
					if (compare(this, user_data, data) != 0) {
						continue;
					} else {
						/*调用用户自己的添加函数*/
						if (insert != NULL) {
							result = insert(this, user_data, data);
							if (result != 0 ) {
								/*添加完成( > 0 )，或者添加失败( < 0 )，返回*/
								goto end;
							} 
						}
						break;
					}
					data += hash_table_p->item_size;
				}
			}
			if (current->valid_count != hash_table_p->item_num) {
				i = current->valid_count;
				current->valid_count++;
//				printf("insert on bucket %d,current %p, valid_count %d\n", hash, current, current->valid_count);
				free_pos_p = (void *)(current + 1) + (uint64_t)i * hash_table_p->item_size;
				break;
			}
			pre = current;
			current = next;
		} while((next != NULL));
	} else {
		/*if free_pos_p != NULL*/
		/*调用用户自己的添加函数*/
		data = free_pos_p;
		if (insert != NULL) {
			result = insert(this, user_data, data);
			if (result != 0 ) {
				/*添加完成( > 0 )，或者添加失败( < 0 )，返回*/
				goto end;
			} 
		}
		/*如果insert返回为0，那么仍旧插入新节点*/
		current->valid_count++;
	}
	
	if (free_pos_p == NULL) {
		if (bucket_alloc == NULL) {
			result =  -EWX_NO_SPACE_ERROR;
			goto end;
		}
		current = bucket_alloc(hash_table_p->bucket_size);
		if (current == NULL) {
			result =  -EWX_NO_SPACE_ERROR;
			goto end;
		}
		pre->next_offset = (uint32_t)(cvmx_ptr_to_phys(current) >> HASH_TABLE_ADDR_SHIFT);
		current->next_offset = 0;
		current->valid_count = 1;
		free_pos_p = (void *)(current+1);
	}
	
	memcpy(free_pos_p, this, hash_table_p->item_size);
end:	
	if (hash_table_p->lock != NULL) {
		cvmx_spinlock_unlock(&hash_table_p->lock[hash]);
	}
	return result;
}

int32_t ewx_hash_table_remove(ewx_hash_table_t *hash_table_p, uint32_t hash, ewx_hash_table_compare_handle_t compare, void *this,
							  void *user_data, ewx_hash_table_remove_handle_t remove, ewx_hash_table_bucket_free_handle_t bucket_free)
{
	ewx_bucket_hd_t *current, *next, *pre;
	ewx_bucket_hd_t *remove_block, *last_block, *pre_last_block, *block_head;
	int i, result = 0;
	void *data;
	int remove_flag, last_flag;
	int last_entry;
	int remove_entry;

	remove_flag = remove_entry = 0;
	last_flag = last_entry = 0;
	remove_block = last_block = NULL;
	

	if (hash_table_p->lock != NULL) {
		cvmx_spinlock_lock(&hash_table_p->lock[hash]);
	}
	
	current = (ewx_bucket_hd_t *)((void *)hash_table_p->base_ptr + (uint64_t)(hash_table_p->bucket_size) * hash);
	block_head = current;
	pre = pre_last_block = current;

	do {
		if (current->next_offset != 0) {
			next = (ewx_bucket_hd_t *)cvmx_phys_to_ptr(((uint64_t)current->next_offset) << HASH_TABLE_ADDR_SHIFT);
		} else {
			next = NULL;
		}
		data = (void *)(current + 1);/*数据开头紧跟着bucket头之后*/		
		for (i=0; i<current->valid_count; i++) {
			if (compare(this, user_data, data) == 0) {
				/*调用用户自己的释放函数*/
				if (remove != NULL) {
					result = remove(this, user_data, data);
					if (result != 0 ) {
						/*释放完成( > 0 )，或者释放失败( < 0 )，返回*/
						goto end;
					} 
				}
				/*必然匹配，并且需要真正删除节点*/
				remove_block = current;
				remove_entry = i;
				
				if (current->next_offset == 0) {
					pre_last_block = pre;
					last_block = current;
					last_entry = current->valid_count - 1;
					last_flag = 1;
				}
				remove_flag = 1;
				break;
			}
			data += hash_table_p->item_size;
		}
		if (remove_flag) {
			break;
		}
		pre = current;
		current = next;
	} while((next != NULL));

	if (!remove_flag) {
		result = -EWX_ITEM_NOT_FOUND;
		goto end;
	}

	if (last_flag == 0) {
		/*当前删除的节点不是最后一个block里的，那么需要找到最后一个节点，并搬移到删除节点的位置*/
		do {
			if (current->next_offset != 0) {
				next = (ewx_bucket_hd_t *)cvmx_phys_to_ptr(((uint64_t)current->next_offset) << 7);
			} else {
				next = NULL;
			}
			if (current->next_offset == 0) {
				pre_last_block = pre;
				last_block = current;
				last_entry = current->valid_count - 1;
				last_flag = 1;
				break;
			}
			pre = current;
			current = next;
		} while ((next != NULL) && (last_flag == 0)) ;
	}

	if ( (remove_block != last_block) || (remove_entry != last_entry)) {
		void *src, *dst;
		dst = (void *)(remove_block +1) + (uint64_t)remove_entry * hash_table_p->item_size;
		src = (void *)(last_block+1) + (uint64_t)last_entry * hash_table_p->item_size;
		memcpy(dst, src, hash_table_p->item_size);
	}
	if ( (last_entry == 0) && (last_block != block_head)) {
		pre_last_block->next_offset = 0;
		if (bucket_free) {
			bucket_free(last_block);
		}
	} else {
		last_block->valid_count --;
	}
end:
	if (hash_table_p->lock != NULL) {
		cvmx_spinlock_unlock(&hash_table_p->lock[hash]);
	}
	return result;
}

void ewx_hash_table_show(ewx_hash_table_t *hash_table_p, ewx_hash_table_show_handle_t show)
{
	uint32_t i, j;
	ewx_bucket_hd_t *current, *next;
	void *data;
	int max_buckets = 0;
	int buckets;

	if (show == NULL)
		return ;

	for (i=0; i<hash_table_p->bucket_num; i++) {
		if (hash_table_p->lock != NULL) {
			cvmx_spinlock_lock(&hash_table_p->lock[i]);
		}
		current = (ewx_bucket_hd_t *)((void *)hash_table_p->base_ptr + (uint64_t)(hash_table_p->bucket_size) * i);
		buckets = 1;
		do {
			if (current->next_offset != 0) {
				next = (ewx_bucket_hd_t *)cvmx_phys_to_ptr(((uint64_t)current->next_offset) << HASH_TABLE_ADDR_SHIFT);
				buckets ++;
			} else {
				next = NULL;
			}
			data = (void *)(current + 1);/*数据开头紧跟着bucket头之后*/		
			for (j=0; j<current->valid_count; j++) {
				show(data);
				data += hash_table_p->item_size;
			}
			current = next;
		} while(next != NULL); 
		if (buckets >= max_buckets) {
			max_buckets = buckets;
		}
		
		if (hash_table_p->lock != NULL) {
			cvmx_spinlock_unlock(&hash_table_p->lock[i]);
		}		
	}
	printf("max_buckets=%d\n", max_buckets);
}


