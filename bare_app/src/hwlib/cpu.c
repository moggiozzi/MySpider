#include "cpu.h"

uint32_t get_cpu_idx(void)
{
    uint32_t affinity;
#if   defined(__ARMCOMPILER_VERSION)
    __asm ("MRC p15, 0, %[affinity], c0, c0, 5" : [affinity] "=r" (affinity));
#elif defined(__ARMCC_VERSION)
    __asm("MRC p15, 0, affinity, c0, c0, 5");
#else
    __asm ("MRC p15, 0, %0, c0, c0, 5" : "=r" (affinity));
#endif
    return affinity & 0xFF;
}
