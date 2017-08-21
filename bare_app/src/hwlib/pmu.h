#ifndef PMU_H_
#define PMU_H_

#ifndef __HWLIB_H__
#include "hwlib.h"
#endif

// Init Performace Monitor Unit
// For more info see the ARM Architecture Reference Manual and Technical Reference Manual
ALT_STATUS_CODE init_pmu(int32_t do_reset, int32_t enable_divider);

// Get Cycle Count from PMU
unsigned int get_pmu_cyclecount (void);

#endif
