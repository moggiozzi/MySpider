#ifndef INTERRUPTS_H_
#define INTERRUPTS_H_

#ifndef __HWLIB_H__
#include <hwlib.h>
#endif
#ifndef __ALT_INT_COMMON_H__
#include <alt_interrupt_common.h>
#endif

// Initializes and enables the interrupt controller.
ALT_STATUS_CODE init_interrupts(void);

ALT_STATUS_CODE socfpga_int_start(ALT_INT_INTERRUPT_t int_id,
                                  alt_int_callback_t callback,
                                  void * context);

#endif
