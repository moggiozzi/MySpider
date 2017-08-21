#ifndef CPU_H_
#define CPU_H_

#ifndef _STDINT_H
#include <stdint.h>
#endif

// Получить индекс текущего ядра процессора
uint32_t get_cpu_idx(void);

#endif
