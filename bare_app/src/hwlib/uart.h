#ifndef UARTH_H_
#define UARTH_H_

#include <hwlib.h>

ALT_STATUS_CODE init_uart(void);
void uart_putc_polled(char c);
char uart_getchar_polled(void);

#endif
