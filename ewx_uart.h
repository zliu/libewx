#ifndef __UART_H__
#define __UART_H__

void uart_write_byte( int uart_index, uint8_t ch );
int uart_printf( int uart_index, const char *format, ... );
uint8_t uart_read_byte_wait( int uart_index );
void uart_wait_idle( int uart_index );
int uart_read_byte_nowait( int uart_index );

/* Use our version of printf instead of the C libraries. We don't
    want the per core banners */
#undef printf
#define printf( format, ... ) uart_printf( 0, format, ##__VA_ARGS__ )
#undef getchar
#define getchar() uart_read_byte_wait( 0 )
#undef getchar_nowait
#define getchar_nowait() uart_read_byte_nowait( 0 )
#undef putchar
#define putchar( ch ) uart_write_byte( 0, ch )

#endif
