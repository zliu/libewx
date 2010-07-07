#ifndef __EWX_DEBUG__
#define __EWX_DEBUG__

int ewx_printd(uint8_t level, int uart_index, const char *format, ... );
uint8_t ewx_debug_level_query();
int ewx_debug_init();
int ewx_dump_work(uint8_t level, cvmx_wqe_t *work);

#undef printd
#define printd( level, format, ... ) ewx_printd( level, 0, format, ##__VA_ARGS__ )

#define debug_code(level, code) \
    do { \
        if (ewx_debug_level_query() > level) { \
            code \
        } \
    } while (0)

#endif
