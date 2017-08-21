#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>
#include <stdarg.h>


#if defined(DEBUG) || defined(USE_SERIAL_PORT)

#define log_printf(...) printf(__VA_ARGS__)

#if defined(USE_SERIAL_PORT) && defined(BUFFERED_SERIAL_PORT)
extern void uart_printf( const char* formatString, ... );
#define printf(...) uart_printf(__VA_ARGS__)
#endif

#else
#define log_printf(...) (void)(0)
#endif

#endif
