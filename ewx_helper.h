#ifndef __EWX_HELPER_H__
#define __EWX_HELPER_H__

#include <cvmx.h>
#include <cvmx-wqe.h>

/** @addtogroup common_defines 常用宏定义  */
//@{ 
/**
 *@brief 让x以a对齐，常用在字节对齐
*/
#define align(x, a) (((x) + (a) - 1) & ~((a) - 1)) 

/**
 *@brief 取一个2字节数的高字节
*/
#define HIGH_BYTE(value) ((value & 0xff00) >> 8) 

/**
 *@brief 取一个2字节数的低字节
*/
#define LOW_BYTE(value) (value & 0xff) 

/**
 *@brief 取1s对应的CPU周期数
*/
#define SEC_CYCLE_COUNT (cvmx_sysinfo_get()->cpu_clock_hz)

/**
 *@brief KB对应的值
*/
#define KB  (1024) 

/**
 *@brief MB对应的值
*/
#define MB  (1024*KB) 

/**
 *@brief 交换a,b的值
*/
#define swap(a, b) \
	do { typeof(a) __tmp = (a); (a) = (b); (b) = __tmp; } while (0) 

/**
 *@brief 根据数据结构中某个member的指针计算出指向该数据结构头部的指针
*/
#define container_of(ptr, type, member) ({                      \
			const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
			(type *)( (char *)__mptr - offsetof(type,member) );})

//@} 常用宏定义

/** @addtogroup helper_funcs 常用辅助函数  */
//@{ 
/** 
 * 把一个32位的ipv4地址转换成一个字符串，形式为XXX.XXX.XXX.XXX
 * @param src 指向32位ipv4地址的指针
 * @param dst 已经分配好空间的字符串指针，在函数调用后该改写该值
 * 
 * @return 指向ipv4地址的字符串指针
 */
const char *ewx_inet_ntop(const uint32_t *src, char *dst);

/** 
 * 把一个形式为XXX.XXX.XXX.XXX的ipv4地址转换成一个整型值
 * @param src 字符串描述的ipv4地址
 * @param dst 最终返回的ipv4整形地址
 * 
 * @return 0，转换成功；1，转换失败；
 */
int32_t ewx_inet_pton(const char *src, uint32_t *dst);

/** 
 * 获取work指向的buffer指针
 * @param work work的指针
 * 
 * @return buffer头部的指针（注：并非数据包的头部）
 */
static inline void *ewx_helper_packet_buffer_head_get(cvmx_wqe_t *work)
{
	return cvmx_phys_to_ptr(((work->packet_ptr.s.addr >> 7) - work->packet_ptr.s.back) << 7);
}

#ifndef __linux__

static inline void sleep(int secs)
{
	uint64_t cycle = cvmx_get_cycle();
	uint64_t end = (unsigned long long)secs * (SEC_CYCLE_COUNT);
	while((cvmx_get_cycle() - cycle) <= end);
}

static inline void msleep(int msecs)
{
	uint64_t cycle = cvmx_get_cycle();
	uint64_t end = ((unsigned long long)msecs * SEC_CYCLE_COUNT)/1000;
	while((cvmx_get_cycle() - cycle) <= end);
}


static inline void usleep(int usecs)
{
	uint64_t cycle = cvmx_get_cycle();
	uint64_t end = ((unsigned long long)usecs * SEC_CYCLE_COUNT)/1000000;
	while((cvmx_get_cycle() - cycle) <= end);
}
#endif
//@} 常用辅助类函数

#endif
