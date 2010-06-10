#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//#include "cvmx-config.h"
#include "cvmx.h"
#include "cvmx-spinlock.h"
//#include "cvmx-fpa.h"
//#include "cvmx-pip.h"
//#include "cvmx-ciu.h"
//#include "cvmx-ipd.h"
//#include "cvmx-pko.h"
//#include "cvmx-dfa.h"
//#include "cvmx-pow.h"
//#include "cvmx-gmx.h"
//#include "cvmx-asx.h"
//#include "cvmx-sysinfo.h"
//#include "cvmx-coremask.h"
#include "cvmx-bootmem.h"
//#include "cvmx-helper.h"
#include "octeon-pci-console.h"
#include "ewx_uart.h"

static CVMX_SHARED cvmx_spinlock_t uart_printf_lock = { CVMX_SPINLOCK_UNLOCKED_VAL };
static CVMX_SHARED uint64_t pci_cons_desc_addr;

static CVMX_SHARED int pci_console;
/**
 * Put a single byte to uart port.
 *
 * @param uart_index Uart to write to (0 or 1)
 * @param ch         Byte to write
 */
inline void uart_write_byte( int uart_index, uint8_t ch )
{
	if ( !pci_console )
	{
		cvmx_uart_lsr_t lsrval;

		// Spin until there is room
		do
		{
			lsrval.u64 = cvmx_read_csr( CVMX_MIO_UARTX_LSR( uart_index ) );
			if ( lsrval.s.thre == 0 )
			{
				cvmx_wait( 10000 );   /* Just to reduce the load on the system */
			}
		}
		while ( lsrval.s.thre == 0 );

		// Write the byte
		cvmx_write_csr( CVMX_MIO_UARTX_THR( uart_index ), ch );
		return;
	}
	else
	{
		char r = '\r';
		if ( pci_cons_desc_addr )
		{
    	    if (ch == '\n')
				octeon_pci_console_write(pci_cons_desc_addr, 0,  &r, 1, 0);
			octeon_pci_console_write(pci_cons_desc_addr, 0,  (char *)&ch, 1, 0);
		}
		else
		{
			printf( "pci_console read error\n" );
		}
	}
}


/**
 * Wait for the TX buffer to be empty
 *
 * @param uart_index Uart to check
 */
void uart_wait_idle( int uart_index )
{
	cvmx_uart_lsr_t lsrval;

	// Spin until there is room
	do
	{
		lsrval.u64 = cvmx_read_csr( CVMX_MIO_UARTX_LSR( uart_index ) );
		if ( lsrval.s.temt == 0 )
		{
			cvmx_wait( 10000 );   // Just to reduce the load on the system
		}
	}
	while ( lsrval.s.temt == 0 );
}


/**
 * Version of printf for direct uart output. This bypasses the
 * normal per core banner processing.
 *
 * @param uart_index Uart to write to
 * @param format     printf format string
 * @return Number of characters written
 */
int uart_printf( int uart_index, const char *format, ... ) __attribute__ ( ( format( printf, 2, 3 ) ) );
int uart_printf( int uart_index, const char *format, ... )
{
	char buffer[ 1024 ];
	va_list args;

	va_start( args, format );
	int result = vsnprintf( buffer, sizeof( buffer ), format, args );
	va_end( args );

	int i = result;
	char *ptr = buffer;

	cvmx_spinlock_lock( &uart_printf_lock );
	while ( i > 0 )
	{
		if ( *ptr == '\n' )
		{
			uart_write_byte( uart_index, '\r' );
		}
		uart_write_byte( uart_index, *ptr );
		ptr++;
		i--;
	}
	cvmx_spinlock_unlock( &uart_printf_lock );

	return result;
}

int uart_prints( int uart_index, char *buffer, int len)
{
	cvmx_spinlock_lock( &uart_printf_lock );
	while ( len > 0 ) {
		if ( *buffer == '\n' ) {
			uart_write_byte( uart_index, '\r' );
		}
		uart_write_byte( uart_index, *buffer );
		buffer++;
		len--;
	}
	cvmx_spinlock_unlock( &uart_printf_lock );
	return len;
}


/**
 * Get a single byte from serial port.
 *
 * @param uart_index Uart to read from (0 or 1)
 * @return The byte read
 */

int ewx_uart_init()
{
	cvmx_bootmem_named_block_desc_t *pci_console_ptr = cvmx_bootmem_phy_named_block_find( "__pci_console", 0 );
	if ( pci_console_ptr == NULL ){
		printf( "pci_consle_ptr == NULL\n" );
		return 0;
	}
	else
		pci_cons_desc_addr = pci_console_ptr->base_addr;
	pci_console = 1;
	printf("find pci_console in 0x%llx\n", (unsigned long long)pci_cons_desc_addr);
	return 0;
}

int uart_read_byte_nowait( int uart_index )
{
	/* Read and return the data. Zero will be returned if there is
	no data */
	if( !pci_console )
	{
		cvmx_uart_lsr_t lsrval;
		lsrval.u64 = cvmx_read_csr( CVMX_MIO_UARTX_LSR( uart_index ) );
		if ( lsrval.s.dr )
		{
			return cvmx_read_csr( CVMX_MIO_UARTX_RBR( uart_index ) );
		}
		else
		{
			return -1;
		}
	}
	else
	{
		char c;
		if ( !pci_cons_desc_addr )
		{
			return 0;
		}
		octeon_pci_console_read(pci_cons_desc_addr, 0,	&c, 1, 1);
		return c;
	}
}

/**
 * Get a single byte from serial port.
 *
 * @param uart_index Uart to read from (0 or 1)
 * @return The byte read
 */
inline uint8_t uart_read_byte_wait( int uart_index )
{
	/* Read and return the data. Will not be returned if there is
	no data */
	if( !pci_console )
	{
		cvmx_uart_lsr_t lsrval;
		while ( 1 )
		{
			lsrval.u64 = cvmx_read_csr( CVMX_MIO_UARTX_LSR( uart_index ) );
			if ( lsrval.s.dr )
			{
				return cvmx_read_csr( CVMX_MIO_UARTX_RBR( uart_index ) );
			}
			return getchar();
		}
	}
	else
	{
		char c;
		while ( 1 )
		{
			if ( !pci_cons_desc_addr )
			{
				return 0;
			}
			octeon_pci_console_read(pci_cons_desc_addr, 0,	&c, 1, 1);
			return c;
		}
	}
}
