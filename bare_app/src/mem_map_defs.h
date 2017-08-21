#ifndef MEM_MAP_DEFS_H_
#define MEM_MAP_DEFS_H_

#include <stdint.h>

// ������ � ����� ������� ������ ���� ������ 4KiB, 64KiB, 1MiB ��� 16MiB (��. alt_pt.c, alt_mmu.h)
// !!! �������� !!!
// 1) ��� ��������� MEMORY_COMMON_END_ADDR ������� ����������������� ��������� ������� � scatter.scat
// 2) ��� ��������� MEMORY_SHARED_BUF_ADDR ��� MEMORY_SHARED_BUF_SIZE ������� ������� ����������� ��������� � ��������� HOST'a
// 3) ��� ��������� MEMORY_EXCEPTION_DIAG_ADDR �������� ����� ������ ����������� � alt_interrupt_armcc.s
//#define MEM_SIZE_3GB
#ifdef MEM_SIZE_3GB
#else // MEM_SIZE_1GB
enum{
  // 1�� ���
  MEMORY_END_ADDR = 0x40000000,
  // ������� ����� ��� BareMetal (��������� 256MB). ��� ��������� ���������� ��������������� ��������� �������� � scatter.scat
  MEMORY_BASE_ADDR = 0x30000000,
  // 8�� ������������ ������� ��� DMA: sdram-qspi, sdram-mmc, sdram-fpga (DMA ����������)
  MEMORY_DMA_BUF_SIZE = 0x1000000,
  MEMORY_DMA_BUF_ADDR = MEMORY_END_ADDR - MEMORY_DMA_BUF_SIZE,
  // ����� ������ 16 �� ��� ������ CPU0-CPU1
  MEMORY_SHARED_BUF_SIZE = 0x1000000,
  MEMORY_SHARED_BUF_ADDR = MEMORY_DMA_BUF_ADDR - MEMORY_SHARED_BUF_SIZE,
  // ������ ������ shared �������� "�������" ������� ����������, � ��� ����� MEMORY_EXCEPTION_DIAG
  MEMORY_COMMON_END_ADDR = MEMORY_SHARED_BUF_ADDR,
};
#endif

#endif

