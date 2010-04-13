#ifndef __EWX_LOG_H__
#define __EWX_LOG_H__
#include <cvmx.h>

extern CVMX_SHARED int ewx_log_level;

#define EWX_LOG_LEVEL_NONE   0
#define EWX_LOG_LEVEL_ERROR  2
#define EWX_LOG_LEVEL_DEBUG  8
#define EWX_LOG_LEVEL_ALL    9

/** @addtogroup log_debug 打印调试函数*/
//@{

/** 
 * 打印调试信息，默认的打印级别为8
 * 如果当前打印级别小于调试级别，将不打印
 * 
 * @param fmt 字符串格式
 * @param args 打印参数
 * 
 */
#define ewx_debug(fmt, args...)											\
    do {																\
		if (ewx_log_level >= EWX_LOG_LEVEL_DEBUG)							\
	        printf(fmt, ##args);										\
    } while(0)

/** 
 * 打印错误信息, 默认的打印级别为2；
 * 如果当前打印级别小于错误级别，将不打印
 * 
 * @param fmt 字符串格式
 * @param args 打印参数
 * 
 */
#define ewx_error_print(fmt, args...)									\
    do {																\
		if (ewx_log_level >= EWX_LOG_LEVEL_ERROR) {							\
	        printf("Error %s() : ", __func__);							\
    	    printf(fmt, ##args);										\
		}																\
    } while(0)

/** 
 * 如果条件不满足，返回错误值，并打印不满足的条件
 * 如果当前打印级别小于错误级别，将不打印
 * 
 * @param statement 判断条件 
 * @param rv 返回的错误值
 * 
 */
#define ewx_if_error_return(statement, rv)						\
	do {														\
		if (!(statement)) {										\
			ewx_error_print(" %s not satisfied\n", #statement);	\
			return rv;											\
		}														\
	} while(0)

/** 
 * 打印信息到串口
 * 
 * @param fmt 字符串格式
 * @param args 打印参数
 */
#define ewx_info(fmt, args...)								\
	do {													\
		printf(fmt, ##args);								\
	} while(0)


#endif

//@} 打印调试函数
