#ifndef __EWX_THREAD_H__
#define __EWX_THREAD_H__

#define EWX_THREAD_MAGIC_NUM 0x0512
#define EWX_THREAD_WORK_UNUSED 0x10 /*用work的unused字段来标识work是一个创建的线程*/

#include "cvmx.h"
#include "cvmx-wqe.h"

typedef struct ewx_thread ewx_thread_t;
typedef void (*ewx_thread_fn)(ewx_thread_t *, void *);

/// ewx 线程内部处理数据结构，一般用户不需要操作该数据结构
struct ewx_thread {
	uint32_t magic;				/**< magic number，以防止意外的work进入 */
	uint16_t free;				/**< 释放当前work的标志 */
	uint16_t tick;				/**< timer的时候使用，单位为毫秒 */
	ewx_thread_fn fn;				/**< 回调函数 */
	void *param;				/**< 回调的参数 */
};

/** @addtogroup thread_function 线程操作函数 */
//@{
/**
 * 线程创建函数，和操作系统下的创建函数不一样，需要考虑调度的一些参数
 *
 * @param tag 线程的tag值，用来同步互斥使用
 * @param tag_type tag类型:CVMX_POW_TAG_TYPE_ORDERED（保序，默认）,CVMX_POW_TAG_TYPE_ATOMIC（原子）,CVMX_POW_TAG_TYPE_NULL
 * @param qos 输入队列
 * @param group 提交的组，一般填写(status_info_g.ipd_group)
 * @param fn 线程回调函数
 * @param param 线程回调传入的参数指针（支持堆栈的指针，内部支持拷贝
 * @param param_len 回调参数的长度（目前支持只80 bytes以下）
 *
 * @return 0，成功；否则，返回错误码
 */
int32_t ewx_thread_create(uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t qos, uint64_t group, ewx_thread_fn fn,
								 void *param, uint32_t param_len);

/**
 * timer创建函数，注意：这里会有隐性的循环
 *
 * @param tag 线程的tag值，用来同步互斥使用
 * @param tag_type tag类型:CVMX_POW_TAG_TYPE_ORDERED（保序，默认）,CVMX_POW_TAG_TYPE_ATOMIC（原子）,CVMX_POW_TAG_TYPE_NULL
 * @param qos 输入队列
 * @param grp 提交的组，一般填写(status_info_g.ipd_group)
 * @param fn 线程回调函数
 * @param param 线程回调传入的参数指针（支持堆栈的指针，内部支持拷贝
 * @param param_len 回调参数的长度（目前支持只80 bytes以下）
 * @param delay timer的延时，单位为毫秒，最大支持1000毫秒
 *
 * @return 0，成功；否则，返回错误码
 */
int32_t ewx_timer_create(uint32_t tag, cvmx_pow_tag_type_t tag_type, uint64_t qos, uint64_t grp, ewx_thread_fn fn,
								void *param, uint32_t param_len, uint16_t delay);

/**
 * 线程处理函数，在主流程work处理函数中被调用
 *
 * @param wqe_p 一般在主处理线程中调用该函数，以调用线程回调函数，普通引擎用户不需要关心该函数；
 *
 * @return 0，成功；否则，返回错误码
 */
int32_t ewx_thread_process(cvmx_wqe_t *wqe_p);

/**
 * 线程终止函数，在创建timer的回调函数中，有可能在达到某种条件后，需要终止该线程，则需要调此函数
 *
 * @param p 线程内部数据结构，一般用户无须直接访问
 *
 * @return 0
 */
int32_t ewx_thread_stop(ewx_thread_t *p);


/**
 * @brief 计时器初始化函数，计时器时钟每隔tick微秒跳动一次，计时间隔最小为tick微秒，
 *        最长间隔为tick*max_ticks微秒；
 *        调用该函数之前需要先为timer分配内存空间，也即CVMX_FPA_TIMER_POOL，
 *        其大小应该不小于同时运行的计时器的数目；
 *
 * @param tick 计时器的时钟间隔，单位：微秒
 * @param max_ticks 计时器的最大计时间隔，单位：微秒*tick
 *                  最小值为 2
 *                  最大值为 (CVMX_TIM_NUM_BUCKETS - 1)
 *
 * @return 0，成功；否则，返回错误码
 */
int32_t ewx_timer_init(uint64_t tick, uint64_t max_ticks);
//@}
#endif
