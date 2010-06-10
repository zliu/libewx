#ifndef __EWX_DEBUG__
#define __EWX_DEBUG__

extern int ewx_printd(uint8_t level, int uart_index, const char *format, ... );
extern uint8_t ewx_debug_level_query();
extern int ewx_debug_init();

#undef printd
#define printd( level, format, ... ) ewx_printd( level, 0, format, ##__VA_ARGS__ )

#define debug_code(level, code) \
    do { \
        if (ewx_debug_level_query() > level) { \
            code \
        } \
    } while (0)
#endif
