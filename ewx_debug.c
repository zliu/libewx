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
#include "cvmx-coremask.h"
#include "cvmx-wqe.h"
#include "cvmx-spinlock.h"

#include "ewx_shell.h"
#include "ewx_helper_util.h"

extern int uart_prints( int uart_index, char *buffer, int len);

static CVMX_SHARED uint8_t debug_level = 1;

#define EWX_DEBUG_CORE_FREE         0
#define EWX_DEBUG_CORE_PROCESSING   1
static CVMX_SHARED cvmx_wqe_t *ewx_debug_work_on_core[32] = {NULL};
static CVMX_SHARED uint8_t ewx_debug_core_status[32] = {EWX_DEBUG_CORE_FREE};
static CVMX_SHARED cvmx_spinlock_t ewx_debug_core_status_lock[32];

int ewx_debug_printd(uint8_t level, int uart_index, const char *format, ... ) __attribute__ ( ( format( printf, 3, 4 ) ) );
int ewx_debug_printd(uint8_t level, int uart_index, const char *format, ... )
{
 	char buffer[ 1024 ];
	va_list args;
    int result;

    if (debug_level >= level) {
        va_start( args, format );
        result = vsnprintf( buffer, sizeof( buffer ), format, args );
        va_end( args );
        uart_prints(uart_index, buffer, result);
        return result;
    }
    return 0;
}

static void __set_debug_level_shcmd(int argc, char *argv[])
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


int ewx_debug_dump_packet(uint8_t level, cvmx_buf_ptr_t buffer_ptr, uint64_t len)
{
    if (debug_level >= level) {
        ewx_helper_dump_packet(buffer_ptr, len);
    }
    return 0;
}

int ewx_debug_dump_work(uint8_t level, cvmx_wqe_t *work)
{
    if (debug_level >= level) {
        ewx_helper_dump_work(work);
    }
    return 0;
}

void ewx_debug_work_in(cvmx_wqe_t *work)
{
    uint8_t core_num = cvmx_get_core_num();
    ewx_debug_work_on_core[core_num] = work;
    cvmx_spinlock_lock(&ewx_debug_core_status_lock[core_num]);
    ewx_debug_core_status[core_num] = EWX_DEBUG_CORE_PROCESSING;
    cvmx_spinlock_unlock(&ewx_debug_core_status_lock[core_num]);
}

void ewx_debug_work_out()
{
    uint8_t core_num = cvmx_get_core_num();
    cvmx_spinlock_lock(&ewx_debug_core_status_lock[core_num]);
    ewx_debug_core_status[core_num] = EWX_DEBUG_CORE_FREE;
    cvmx_spinlock_unlock(&ewx_debug_core_status_lock[core_num]);
}

static int __core_alive(uint8_t core_num)
{
    int i, free = 0;
    for (i = 0; i < 100; i ++) {
        cvmx_spinlock_lock(&ewx_debug_core_status_lock[core_num]);
        if (ewx_debug_core_status[core_num] == EWX_DEBUG_CORE_FREE) {
            free++;
        }
        cvmx_spinlock_unlock(&ewx_debug_core_status_lock[core_num]);
    }
    if (free > 0) {
        return 1;
    } else {
        return 0;
    }
}

int ewx_debug_dump_dead_work()
{
    uint32_t coremask = cvmx_sysinfo_get()->core_mask;
    int i;
    for (i = 0; i < 32; i++) {
        if ((cvmx_coremask_core(i) & coremask) != 0) {
            if (!__core_alive(i)) {
                cvmx_dprintf("Core #%u dead\n", i);
                ewx_debug_dump_work(0, ewx_debug_work_on_core[i]);
            }
        }
    }
    return 0;
}

static void __dump_dead_work_shcmd(int argc, char *argv[])
{
    ewx_debug_dump_dead_work();
}

void ewx_debug_init()
{
    int i;
    if (ewx_shell_status_check() == 1) {
        ewx_shell_cmd_register("dlevel", "set / query debug level", __set_debug_level_shcmd);
        ewx_shell_cmd_register("dd", "dump work on dead core", __dump_dead_work_shcmd);
    }
    for (i = 0; i < 32; i++) {
        cvmx_spinlock_init(&ewx_debug_core_status_lock[i]);
    }
}

