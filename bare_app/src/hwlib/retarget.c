#include <stdio.h>

#if defined(STANDALONE) || defined(USE_SERIAL_PORT)

/* Importing __use_no_semihosting ensures that our image doesn't link
 * with any C Library code that makes direct use of semihosting.
 */
#pragma import(__use_no_semihosting)
FILE __stdout;
void _sys_exit(int return_code)
{
    while(1);
}

#if !defined(USE_SERIAL_PORT) || defined(BUFFERED_SERIAL_PORT)
int fputc(int ch, FILE *f)
{
    return 0;
}
int fgetc(FILE *f)
{
    return 0;
}

void _ttywrch(int ch)
{
}
int __backspace(FILE *f)
{
    return 1;
}
#endif

#endif

#ifdef USE_SERIAL_PORT
#ifndef BUFFERED_SERIAL_PORT
extern void uart_putc_polled(char c);
extern char uart_getchar_polled(void);

/*
** These must be defined to avoid linking in stdio.o from the
** C Library
*/
//FILE __stdout;
//FILE __stdin;

/*
** __backspace must return the last char read to the stream
** fgetc() needs to keep a record of whether __backspace was
** called directly before it
*/
int last_char_read;
int backspace_called;

int fgetc(FILE *f)
{
    unsigned char tempch;
    tempch = uart_getchar_polled();
    last_char_read = (int)tempch;       /* backspace must return this value */
    return tempch;
}

int fputc(int ch, FILE *f)
{
    unsigned char tempch = ch;
    if (tempch == '\n') uart_putc_polled('\r');
    uart_putc_polled(tempch);
    return ch;
}

void _ttywrch(int ch)
{
    unsigned char tempch = ch;
    if (tempch == '\n') uart_putc_polled('\r');
    uart_putc_polled(tempch);
}

/*
** The effect of __backspace() should be to return the last character
** read from the stream, such that a subsequent fgetc() will
** return the same character again.
*/
int __backspace(FILE *f)
{
    backspace_called = 1;
    return 1;
}
#endif // #ifndef BUFFERED_SERIAL_PORT

#endif  // USE_SERIAL_PORT
