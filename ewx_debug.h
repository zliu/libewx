#ifndef __EWX_DEBUG__
#define __EWX_DEBUG__

#include "cvmx-wqe.h"

/* debug_level初始值为1, 编写代码时应把debug语句的level设置大于0, 比如可将错误信息设置为1;
 * 也就是说当debug_level为0时屏蔽所有的打印信息,包括错误
 */
int ewx_debug_printd(uint8_t level, int uart_index, const char *format, ... );
#undef printd
#define printd( level, format, ... ) ewx_debug_printd( level, 0, format, ##__VA_ARGS__ )
uint8_t ewx_debug_level_query();
int ewx_debug_dump_work(uint8_t level, cvmx_wqe_t *work);
int ewx_debug_dump_packet(uint8_t level, cvmx_buf_ptr_t  buffer_ptr, uint64_t len);
#define debug_code(level, code) \
    do { \
        if (ewx_debug_level_query() >= level) { \
            code \
        } \
    } while (0)


#define EWX_DEBUG_CORE_FREE         0
#define EWX_DEBUG_CORE_PROCESSING   1

void ewx_debug_work_in(cvmx_wqe_t *work);
void ewx_debug_work_out();
int ewx_debug_dump_dead_work();
int ewx_debug_init();

#endif
