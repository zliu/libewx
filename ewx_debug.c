/** 
 * @file ewx_debug.c
 * @brief 调试代码工具
 *        printd使用函数而不是宏的原因是,debug_level是库的似有变量,只能在库的函数中访问操作
 * @author Cheng Long
 * @version 1.0
 * @date 2010-06-10
 */
#include "cvmx.h"
#include "cvmx-spinlock.h"
#include "ewx_shell.h"

extern int uart_prints( int uart_index, char *buffer, int len);

static CVMX_SHARED uint8_t debug_level = 9;

int ewx_printd(uint8_t level, int uart_index, const char *format, ... ) __attribute__ ( ( format( printf, 3, 4 ) ) );
int ewx_printd(uint8_t level, int uart_index, const char *format, ... )
{
 	char buffer[ 1024 ];
	va_list args;
    int result;

    if (debug_level <= level) {
        va_start( args, format );
        result = vsnprintf( buffer, sizeof( buffer ), format, args );
        va_end( args );
        uart_prints(uart_index, buffer, result);
        return result;
    }
    return 0;
}

static void set_debug_level_shell_cmd(int argc, char *argv[])
{
    int8_t level;
    if (argc == 1) {
        printf("debug level = %u\n", debug_level);
    } else if (argc == 2) {
        level = (uint8_t)strtoull(argv[1], 0, 10);
        if ((level < 0) || (level > 9))
        {
            printf("Illegal level value.\n");
        } else {
            debug_level = (uint8_t)level;
        }
    } else {
        printf( "Illegal number of parameters.\n" );
    }    
}

uint8_t ewx_debug_level_query()
{
    return debug_level;
}

void ewx_debug_init()
{
    if (ewx_shell_status_check() == 1) {
        ewx_shell_cmd_register("dlevel", "set / query debug level", set_debug_level_shell_cmd);
    }
}
