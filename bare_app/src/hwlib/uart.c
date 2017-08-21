#include "alt_16550_buffer.h"
#include "alt_16550_uart.h"

#include <stdio.h>
#include <stdarg.h>

#ifdef USE_SERIAL_PORT

ALT_16550_HANDLE_t uart_handle;

#ifdef BUFFERED_SERIAL_PORT

ALT_16550_BUFFER_t buffer;
char tmp_str_buf[256];
void uart_printf( const char* formatString, ... )
{
  size_t written, cnt;
  va_list args;
  va_start( args, formatString );
  cnt = vsnprintf( tmp_str_buf, sizeof(tmp_str_buf), formatString, args );
  va_end( args );
  alt_16550_buffer_write_raw( &buffer, tmp_str_buf, cnt, &written );
}

#else

void uart_putc_polled(char c)
{
  uint32_t s;
  if(uart_handle.location==0)
    return;
  do
  {
    alt_16550_line_status_get(&uart_handle,  &s);
  }
  while((s & ALT_16550_LINE_STATUS_TEMT) == 0); //transmitter empty
    alt_16550_write(&uart_handle, c);
}

char uart_getchar_polled(void)
{
  char ch;
  alt_16550_read(&uart_handle, &ch);
  return ch;
}

#endif

// Initializes and enables the 16550 UART. It uses the following
// settings:
//  - 8-N-1
//  - 115200 baud
//
// Also initializes the UART buffer.
//
ALT_STATUS_CODE socfpga_16550_start(ALT_16550_HANDLE_t * handle,
                                    ALT_16550_DEVICE_t device,
                                    void * location, alt_freq_t frequency,
                                    ALT_16550_BUFFER_t * buffer, ALT_INT_INTERRUPT_t int_id, uint32_t int_target)
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;

    // Initialize the UART
    if (status == ALT_E_SUCCESS)
    {
        status = alt_16550_init(device, location, frequency, handle);
        if (status != ALT_E_SUCCESS)
        {
            printf("ERROR: alt_16550_init failed.\n");
        }
    }

    // Configure for 8-N-1.
    // This is not really needed as the default configuration is 8-N-1.
    if (status == ALT_E_SUCCESS)
    {
        status = alt_16550_line_config_set(handle, ALT_16550_DATABITS_8,
                                                   ALT_16550_PARITY_DISABLE,
                                                   ALT_16550_STOPBITS_1);
        if (status != ALT_E_SUCCESS)
        {
            printf("ERROR: alt_16550_line_config_set failed.\n");
        }
    }

    // Configure for 115200 baud.
    if (status == ALT_E_SUCCESS)
    {
        status = alt_16550_baudrate_set(handle, ALT_16550_BAUDRATE_115200);
        if (status != ALT_E_SUCCESS)
        {
            printf("ERROR: alt_16550_baudrate_set(115200) failed.\n");
        }
    }

    if (buffer != 0 )
    {
      // Configure the buffering
      if (status == ALT_E_SUCCESS)
      {
          status = alt_16550_buffer_init(buffer, handle, int_id, int_target);
      }
      if (status == ALT_E_SUCCESS)
      {
          status = alt_16550_buffer_echo_enable(buffer, true, 0);
      }
    }
    // Enable the UART
    if (status == ALT_E_SUCCESS)
    {
        status = alt_16550_enable(handle);
        if (status != ALT_E_SUCCESS)
        {
            printf("ERROR: alt_16550_enable failed.\n");
        }
    }

    return status;
}

//
// Disables and uninitializes the 16550 UART.
//
void socfpga_16550_stop(ALT_16550_HANDLE_t * handle,
                        ALT_16550_BUFFER_t * buffer, ALT_INT_INTERRUPT_t int_id)
{
    // Disable the UART
    if (alt_16550_disable(handle) != ALT_E_SUCCESS)
    {
        printf("WARN: alt_16550_disable failed.\n");
    }

    // Disable the buffering
    if (buffer != 0 && alt_16550_buffer_uninit(buffer, int_id) != ALT_E_SUCCESS)
    {
        printf("WARN: alt_16550_buffer_uninit failed.\n");
    }

    // Uninitialize the UART
    if (alt_16550_uninit(handle) != ALT_E_SUCCESS)
    {
        printf("WARN: alt_16550_uninit failed.\n");
    }
}

// Прерывания должны быть инициализированы socfpga_int_start
ALT_STATUS_CODE init_uart()
{
  ALT_STATUS_CODE status = ALT_E_SUCCESS;
  uint32_t int_priority;

  uart_handle.device     = ALT_16550_DEVICE_SOCFPGA_UART0;
  uart_handle.location   = 0;
  uart_handle.clock_freq = 0;

  // В случае буфферизированного ввода/вывод использовать alt_16550_buffer_write_line() / alt_16550_buffer_read_line()
  // иначе alt_16550_write() / alt_16550_read();
  // Start the UART
  if (status == ALT_E_SUCCESS)
  {
#ifdef BUFFERED_SERIAL_PORT
    status = socfpga_16550_start(&uart_handle, uart_handle.device, uart_handle.location, uart_handle.clock_freq,
                                 &buffer, ALT_INT_INTERRUPT_UART0, 0);
#else
    status = socfpga_16550_start(&uart_handle, uart_handle.device, uart_handle.location, uart_handle.clock_freq,
                                 0, ALT_INT_INTERRUPT_UART0, 0);
#endif
      if (status == ALT_E_SUCCESS)
      {
        alt_int_dist_priority_get(ALT_INT_INTERRUPT_F2S_FPGA_IRQ0, &int_priority);
        int_priority++; // уменьшить приоритет (0-наивысший, 255-наинизший приоритет)
        alt_int_dist_priority_set(ALT_INT_INTERRUPT_UART0, int_priority);
      }
  }
  return status;
}
ALT_STATUS_CODE uninit_uart()
{
  ALT_STATUS_CODE status = ALT_E_SUCCESS;
#ifdef BUFFERED_SERIAL_PORT
  // Wait for the buffer TX to flush.
  if (status == ALT_E_SUCCESS)
  {
      int i = 10000;

      while (--i)
      {
          uint32_t level_tx = 0;
          status = alt_16550_buffer_level_tx(&buffer, &level_tx);

          if (status != ALT_E_SUCCESS)
          {
              break;
          }

          if (level_tx == 0)
          {
              break;
          }
      }

      if (i == 0)
      {
          printf("ERROR: Timeout waiting for buffer TX to flush.\n");
          status = ALT_E_TMO;
      }
  }
#endif
  //
  // Wait for the UART TX to flush.
  //
  if (status == ALT_E_SUCCESS)
  {
      int i = 0;

      while (--i)
      {
          uint32_t line_status;
          status = alt_16550_line_status_get(&uart_handle, &line_status);

          if (status != ALT_E_SUCCESS)
          {
              break;
          }
          if (line_status & ALT_16550_LINE_STATUS_TEMT)
          {
              break;
          }
      }

      if (i == 0)
      {
          printf("ERROR: Timeout waiting for UART TX to empty.\n");
          status = ALT_E_TMO;
      }
  }

#ifdef BUFFERED_SERIAL_PORT
  // Stop the 16550 UART
  socfpga_16550_stop(&uart_handle, &buffer, ALT_INT_INTERRUPT_UART0);
#endif

  if (status == ALT_E_SUCCESS)
  {
      printf("RESULT: Example completed successfully.\n");
  }
  else
  {
      if (status == ALT_E_UART_OE)
      {
          printf("ERROR: UART Overrun Error detected. Bailing.\n");
      }
      else if (status == ALT_E_UART_PE)
      {
          printf("ERROR: UART Parity Error detected. Bailing.\n");
      }
      else if (status == ALT_E_UART_FE)
      {
          printf("ERROR: UART Framing Error detected. Bailing.\n");
      }
      else if (status == ALT_E_UART_RFE)
      {
          printf("ERROR: UART Receiver FIFO Error detected. Bailing.\n");
      }

      printf("RESULT: Some failures detected.\n");
  }
  return status;
}

#endif
