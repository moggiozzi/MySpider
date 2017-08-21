#include "pmu.h"

// Init Performace Monitor Unit
// For more info see the ARM Architecture Reference Manual and Technical Reference Manual
ALT_STATUS_CODE init_pmu(int32_t do_reset, int32_t enable_divider)
{
  // enable all counters
  int32_t value = 1;
  if (do_reset)
  {
    value |= 2;     // reset all counters to zero.
    value |= 4;     // reset cycle counter to zero.
  }
  if (enable_divider)
    value |= 8;     // enable "by 64" divider for CCNT (счетчик будет тикать каждые 64 цикла)
  value |= 16;
#if defined(__ARMCC_VERSION)
  // Performance Monitors Control Register
  __asm volatile ("MCR p15, 0, value, c9, c12, 0");
  // enable all counters:
  __asm volatile ("MCR p15, 0, 0x8000000f, c9, c12, 1");
  // clear overflows:
  __asm volatile ("MCR p15, 0, 0x8000000f, c9, c12, 3");
#else
  __asm volatile ("MCR p15, 0, %[value], c9, c12, 0" : : [value] "r" (value));
  __asm volatile ("MCR p15, 0, %0, c9, c12, 1" :: "r"(0x8000000f));
  __asm volatile ("MCR p15, 0, %0, c9, c12, 3" :: "r"(0x8000000f));
#endif
  return ALT_E_SUCCESS;
}

// Get Cycle Count from PMU
unsigned int get_pmu_cyclecount (void)
{
  unsigned int value=0;
  // Read Cycle Count Register
#if   defined(__ARMCOMPILER_VERSION)
    __asm ("MRC p15, 0, %[value], c9, c13, 0" : [value] "=r" (value));
#elif defined(__ARMCC_VERSION)
    __asm volatile ("MRC p15, 0, value, c9, c13, 0");
#else
    __asm ("MRC p15, 0, %0, c9, c13, 0" : "=r" (value));
#endif
  return value;
}
