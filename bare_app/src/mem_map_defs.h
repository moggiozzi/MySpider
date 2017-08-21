#ifndef MEM_MAP_DEFS_H_
#define MEM_MAP_DEFS_H_

#include <stdint.h>

// Размер и адрес области должны быть кратны 4KiB, 64KiB, 1MiB или 16MiB (см. alt_pt.c, alt_mmu.h)
// !!! ВНИМАНИЕ !!!
// 1) При изменении MEMORY_COMMON_END_ADDR следует подкорректировать настройки линкера в scatter.scat
// 2) При изменении MEMORY_SHARED_BUF_ADDR или MEMORY_SHARED_BUF_SIZE следует сделать аналогичные изменения в программе HOST'a
// 3) При изменении MEMORY_EXCEPTION_DIAG_ADDR изменить адрес буфера диагностики в alt_interrupt_armcc.s
//#define MEM_SIZE_3GB
#ifdef MEM_SIZE_3GB
#else // MEM_SIZE_1GB
enum{
  // 1Гб ОЗУ
  MEMORY_END_ADDR = 0x40000000,
  // Базовый адрес для BareMetal (последние 256MB). При изменении необходимо скорректировать параметры линковки в scatter.scat
  MEMORY_BASE_ADDR = 0x30000000,
  // 8Мб некэшируемой область для DMA: sdram-qspi, sdram-mmc, sdram-fpga (DMA контроллер)
  MEMORY_DMA_BUF_SIZE = 0x1000000,
  MEMORY_DMA_BUF_ADDR = MEMORY_END_ADDR - MEMORY_DMA_BUF_SIZE,
  // Общая память 16 Мб для обмена CPU0-CPU1
  MEMORY_SHARED_BUF_SIZE = 0x1000000,
  MEMORY_SHARED_BUF_ADDR = MEMORY_DMA_BUF_ADDR - MEMORY_SHARED_BUF_SIZE,
  // адреса меньше shared являются "обычной" памятью приложения, в том числе MEMORY_EXCEPTION_DIAG
  MEMORY_COMMON_END_ADDR = MEMORY_SHARED_BUF_ADDR,
};
#endif

#endif

