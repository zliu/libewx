#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//#include "cvmx-config.h"
#include "cvmx.h"
//#include "cvmx-spinlock.h"
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
//#include "cvmx-bootmem.h"
//#include "cvmx-helper.h"

#include "ewx_shell.h"
#include "ewx_uart.h"
// VT102 emulation
#define DEL		0x7f
#define BEL		0x07
#define BS		0x08
#define TAB		0x09
#define ESC		0x1b

#define ETX		0x03
#define CTRL_C	ETX
#define NAK		0x15
#define CTRL_U	NAK
#define ETB		0x17
#define CTRL_W	ETB
#define SOH		0x01
#define CTRL_A	SOH
#define VT		0x0b
#define CTRL_K	VT

#define CMD_NAME_SIZE		32
#define CMD_COMMENT_SIZE	64
#define MAX_ARGV_ARGS		32

#define MAX_CMDS			64
#define MAX_HISTORY_CMDS	64

struct cmd {
    char name[ CMD_NAME_SIZE ];
	char comment[ CMD_COMMENT_SIZE ];
    void ( *func )( int, char *[] );
};

static struct cmd cmd_list[ MAX_CMDS ];

#define CMD_BUF_SIZE		256
static char shell_cmd_buf[ CMD_BUF_SIZE ];
static int shell_cmd_index;
static char *argv[ MAX_ARGV_ARGS ];

static char history_shell_cmd[ MAX_HISTORY_CMDS ][ CMD_BUF_SIZE ];
static int current_history_index = 0;
static int first_history_index = 0;
static int last_history_index = MAX_HISTORY_CMDS - 1;
static int valid_history_entry = 0;
static int browsing_history = 0;
static char shell_cmd_buf_tmp[ CMD_BUF_SIZE ];

static int escape_pressed = 0;
static int escape_seq_len = 0;
static char escape_seq_buf[ 16 ];

#define DEFAULT_PROMPT "PASH # "
char prompt[ 32 ];

int ewx_shell_cmd_register( const char *name, const char *comment, void ( *func )( int, char *[] ) )
{
    int i;
    
    if ( !name || !name[ 0 ] ) {
        printf( "register_shell_cmd: bad shell cmd name.\n" );
        return -1;
    }
    
    for ( i = 0; i < MAX_CMDS; i++ ) {
        if ( strcmp( name, cmd_list[ i ].name ) == 0 ) {
            break;
        }
    }
    if ( i < MAX_CMDS ) {
        printf( "register_shell_cmd: shell cmd [%s] already registered.\n", name );
        return -1;
    }
    
    for ( i = 0; i < MAX_CMDS; i++ ) {
        if ( cmd_list[ i ].name[ 0 ] == 0 ) {
            break;
        }
    }
    if ( i >= MAX_CMDS ) {
        printf( "register_shell_cmd: too more shell cmds.\n" );
        return -1;
    }
    
    strncpy( cmd_list[ i ].name, name, CMD_NAME_SIZE );
	strncpy( cmd_list[ i ].comment, comment, CMD_COMMENT_SIZE );
    cmd_list[ i ].func = func;
    return 0;
}

int ewx_shell_cmd_unregister( const char *name )
{
    int i;
    
    if ( !name || !name[ 0 ] ) {
        printf( "unregister_shell_cmd: bad shell cmd name.\n" );
        return -1;
    }
    
    for ( i = 0; i < MAX_CMDS; i++ ) {
        if ( strcmp( name, cmd_list[ i ].name ) == 0 ) {
            break;
        }
    }
    if ( i >= MAX_CMDS ) { 
        printf( "unregister_shell_cmd: shell cmd [%s] has not registered.\n", name );
        return -1;
    }
    
    cmd_list[ i ].name[ 0 ] = 0;
    return 0;
}

static void execute_cmd( char *buf )
{
    const char *delim = " \t";
    int argc = 0, i = 0;
    
    // Parse command line and setup args
    argv[ argc++ ] = strtok( buf, delim );
    if ( !argv[ argc - 1 ] ) {
        printf("Bad command token??\n");
        return;
    }
    
    while ( 1 ) {
        // Can't handle more than MAX_ARGV_ARGS arguments
        if ( argc >= MAX_ARGV_ARGS ) {
            break;
        }
        
        argv[ argc++ ] = strtok( 0, delim );
        if ( !argv[ argc - 1 ] ) {
            argc--;
            break;
        }
    }
    
    // Search for the command
    for ( i = 0; i < MAX_CMDS; i++ ) {
        if ( strcmp( cmd_list[ i ].name, argv[ 0 ] ) == 0 ) {
            break;
        }
    }
    if ( i >= MAX_CMDS ) {
        printf( "%s: command not found\n", buf );
        return;
    }
    if ( !cmd_list[ i ].func ) {
        printf( "Bad function pointer!\n" );
        return;
    }
    
    // call the command handler
    ( cmd_list[ i ].func )( argc, argv );
}

void print_prompt( void )
{
    printf( prompt );
    fflush( stdout );
}

void ewx_shell_run( void )
{
	int ch;
	do
	{
		ch = getchar_nowait();
		switch ( ch )
		{
			if ( ch != 0 )
				case -1:
					break;
				case 0:
					escape_pressed = 0;
					break;
				case '\r':
				case '\n':
					escape_pressed = 0;
					shell_cmd_buf[ shell_cmd_index ] = 0;
					shell_cmd_index = 0;
					printf( "\n" );

					if ( shell_cmd_buf[ 0 ] != 0 )
					{
						strcpy( shell_cmd_buf_tmp, shell_cmd_buf );
						execute_cmd( shell_cmd_buf_tmp );
						browsing_history = 0;
						if ( ( valid_history_entry > 0 ) &&
							( strcmp( history_shell_cmd[ last_history_index ], shell_cmd_buf ) == 0 ) )
						{
							print_prompt();
							break;
						}
						if ( valid_history_entry < MAX_HISTORY_CMDS )
						{
							last_history_index = ( last_history_index + 1 ) % MAX_HISTORY_CMDS;
							valid_history_entry++;
						}
						else
						{
							last_history_index = ( last_history_index + 1 ) % MAX_HISTORY_CMDS;
							first_history_index = ( first_history_index + 1 ) % MAX_HISTORY_CMDS;
						}
						strcpy( history_shell_cmd[ last_history_index ], shell_cmd_buf );
					}
					print_prompt();
					fflush( stdout );
					break;
				case TAB:
					escape_pressed = 0;
					putchar( BEL );
					fflush( stdout );
					break;
				case CTRL_C:
					escape_pressed = 0;
					shell_cmd_buf[ shell_cmd_index ] = 0;
					shell_cmd_index = 0;
					printf( "\n" );
					print_prompt();
					fflush( stdout );
					break;
				case BS:
				case DEL:
					escape_pressed = 0;
					if ( shell_cmd_index == 0 )
					{
						putchar( BEL );
						fflush( stdout );
					}
					else
					{
						shell_cmd_index--;
						shell_cmd_buf[ shell_cmd_index ] = 0;
						printf( "\b \b" );
						fflush( stdout );
					}
					break;
				case ESC:
					escape_pressed = 1;
					escape_seq_len = 0;
					escape_seq_buf[ escape_seq_len ] = 0;
					break;
				default:
					if ( escape_pressed == 1 )
					{
						escape_seq_buf[ escape_seq_len++ ] = ch;
						escape_seq_buf[ escape_seq_len ] = 0;
						if ( escape_seq_len == 1 )
						{
							if ( escape_seq_buf[ 0 ] != '[' )
							{
								escape_pressed = 0;
							}
						}
						else if ( escape_seq_len == 2 )
						{
							if ( ( escape_seq_buf[ 1 ] != 'A' ) && ( escape_seq_buf[ 1 ] != 'B' ) )
							{
								escape_pressed = 0;
								break;
							}
							int show_history = 0;
							if ( browsing_history == 0 )
							{
								if ( ( escape_seq_buf[ 1 ] == 'A' ) && ( valid_history_entry > 0 ) )
								{
									current_history_index = last_history_index;
									show_history = 1;
									browsing_history = 1;
								}
							}
							else {
								if ( escape_seq_buf[ 1 ] == 'A' )
								{
									if ( current_history_index != first_history_index  )
									{
										current_history_index = ( current_history_index - 1 + MAX_HISTORY_CMDS ) % MAX_HISTORY_CMDS;
										show_history = 1;
									}
								}
								else if ( escape_seq_buf[ 1 ] == 'B' )
								{
									if ( current_history_index != last_history_index  )
									{
										current_history_index = ( current_history_index + 1 + MAX_HISTORY_CMDS ) % MAX_HISTORY_CMDS;
										show_history = 1;
									}
								}
							}
							if ( show_history == 1 )
							{
								int i;
								for ( i = 0; i < shell_cmd_index; i++ )
								{
									printf( "\b \b" );
								}
								strcpy( shell_cmd_buf, history_shell_cmd[ current_history_index ] );
								shell_cmd_index = strlen( shell_cmd_buf );
								printf( "%s", shell_cmd_buf );
							}
							escape_pressed = 0;
						}
						break;
					}
					shell_cmd_buf[ shell_cmd_index++ ] = ch;
					putchar( ch );
					fflush( stdout );
					break;
		}
	} while( 0 );
}

__attribute__( ( noreturn ) ) void shell( void )
{
    int ch;
    int i = 0;
    
    print_prompt();
    while ( 1 ) {
        ch = getchar_nowait();
        if ( ch == -1 ) {
            cvmx_wait( 10000 );
            continue;
        }
        
        putchar( ch );
        shell_cmd_buf[ i++ ] = ch;
        
        if ( ch == '\n' ) {
            shell_cmd_buf[ i - 1 ] = 0;
            if ( shell_cmd_buf[ 0 ] != 0 ) {
                execute_cmd( shell_cmd_buf );
            }
            i = 0;
            print_prompt();
        } else if ( ch == DEL ) {
            if (i != 0 ) {
                i--;
            } else {
                putchar( BEL );
            }
        }
    }
}

void shell_help( int argc, char *argv[] )
{
	int i;
	//int ch;

	printf( "Available Commands: \n\n" );
	for ( i = 0; i < MAX_CMDS; i++ ) {
		if ( cmd_list[ i ].name[ 0 ] == 0 ) {
			continue;
		}
		printf( " %-32s %-s\n", cmd_list[ i ].name, cmd_list[ i ].comment );
		/*if ( ( i != 0 ) && ( ( i % 30 ) == 0 ) ) {
			printf( "Press Enter to Continue or Q to Quit.\n" );
			ch = getchar();
			switch (ch) {
				case 0x0d:
					if ( i >= MAX_CMDS ) {
						return ;
					}
					break;
				case 'q':
				case 'Q':
					return ;
			}
		}*/
	}
}

void change_prompt( int argc, char *argv[] )
{
	int i;
	unsigned char buffer[ 256 ];
	char *ptr;
	
	if ( argc < 2 || !argv[ 1 ] )
	{
		printf( "Bad args, argc = %d, argv[1] = [%s]\n", argc, argv[ 1 ] );
		return;
	}
	ptr = (char *)buffer;
	
	for ( i = 1; i < argc; i++ )
	{
		ptr += sprintf( ptr, "%s", argv[ i ] );
		*ptr++ = 0x20;
	}
	*ptr = 0;

	strncpy( prompt, (char *)buffer, 32 );
}

int ewx_shell_app_init( void )
{
	print_prompt();
	return 0;
}

void ewx_shell_init( void )
{
	int i;

	strncpy( prompt, DEFAULT_PROMPT, 32 );
	for ( i = 0; i < MAX_CMDS; i++ )
	{
		cmd_list[ i ].name[ 0 ] = 0;
	}
	
	ewx_shell_cmd_register( "help", "show all commands", shell_help );
	ewx_shell_cmd_register( "h", "show all commands", shell_help );
	//register_shell_cmd( "prompt", "change prompt", change_prompt );
    ewx_shell_app_init();
}


