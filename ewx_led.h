#ifndef __EWX_LED_H__
#define __EWX_LED_H__

#define EWX_MAX_LED 8

/** @defgroup hardware_function 硬件相关函数*/
//@{
//@}

/** @addtogroup led_function 灯处理函数
 * @ingroup hardware_function
*/
//@{
/** 
 * 点灯初始化函数
 * 
 * 
 * @return 成功，返回0；失败，返回1
 */
int ewx_led_init();

/** 
 * 点灯周期处理函数，经验值为100ms调用一次
 * 
 * 
 * @return 成功，返回0；失败，返回1
 */
int ewx_led_timer_process();

//@}
#endif
