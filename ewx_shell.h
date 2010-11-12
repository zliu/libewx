#ifndef __EWX_SHELL_H__
#define __EWX_SHELL_H__

#include "cvmx.h"

/**@addtogroup shell_function shell操作函数（内部使用）
 * @ingroup hardware_function
 */
//@{
/**
 * shell运行函数，需要被定期调用，推荐值是一毫秒调用一次
 *
 */
void ewx_shell_run( void );

/**
 * shell注册函数
 *
 * @param name 命令名称
 * @param comment 命令的帮助信息
 * @param func 命令回调函数
 *
 * @return 0,注册成功；1，注册失败
 */
int ewx_shell_cmd_register( const char *name, const char *comment, void ( *func )( int, char *[] ) );

/**
 * shell卸载函数
 *
 * @param name 命令名称
 *
 * @return 0，卸载成功；1，卸载失败；
 */
int ewx_shell_cmd_unregister( const char *name );

/**
 * 串口初始化函数，需要在control core上调用
 *
 *
 * @return 0，初始化成功；1，初始化失败；
 */
int ewx_uart_init();

/**
 * shell初始化函数，需要在control core上调用
 *
 */
void ewx_shell_init( void );


/**
 * shell app 初始化函数
 *
 *
 * @return 0，初始化成功；1，初始化失败
 */
int ewx_shell_app_init(void);

uint8_t ewx_shell_status_check();

void ewx_shell_main_loop(void);

#endif

//@}
