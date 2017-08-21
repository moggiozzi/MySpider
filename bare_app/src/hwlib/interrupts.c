#include "interrupts.h"
#include "cpu.h"
#include <alt_interrupt.h>

// Initializes and enables the interrupt controller.
ALT_STATUS_CODE init_interrupts()
{
    ALT_STATUS_CODE status = ALT_E_SUCCESS;
    // Initialize the global and CPU interrupt items
    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_global_init();
    }
    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_cpu_init();
    }
    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_cpu_enable();
    }
    if (status == ALT_E_SUCCESS)
    {
        status = alt_int_global_enable();
    }
    return status;
}

ALT_STATUS_CODE socfpga_int_start(ALT_INT_INTERRUPT_t int_id,
                                  alt_int_callback_t callback,
                                  void * context)
{
  ALT_STATUS_CODE status = ALT_E_SUCCESS;
  // Setup the interrupt specific items
  if (status == ALT_E_SUCCESS)
  {
      status = alt_int_isr_register(int_id, callback, context);
  }
  if (   (status == ALT_E_SUCCESS)
      && (int_id >= 32)) // Ignore target_set() for non-SPI interrupts.
  {
      int target = get_cpu_idx() + 1;//0x2;
      status = alt_int_dist_target_set(int_id, target);
  }
  // Enable the distributor, CPU, and global interrupt
  if (status == ALT_E_SUCCESS)
  {
      status = alt_int_dist_enable(int_id);
  }
  return status;
}
