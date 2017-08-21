#ifndef QSPI_H_
#define QSPI_H_

#ifndef __HWLIB_H__
#include "hwlib.h"
#endif

#include <alt_dma_common.h>
#include <alt_dma.h>

#define WAIT_QSPI_TIMEOUT_MKS (10*1000000) // 10 ������

ALT_STATUS_CODE init_qspi(void);

uint32_t qspi_is_ready(void);
// ��������� �������� ���� ������ � ����������� qspi (������ ��� �������)
void qspi_read_regs(void);
// ������� size ������ �� ������ addr (!!! ������������ �� ������� � 64�� !!!)
ALT_STATUS_CODE qspi_clear(uint32_t addr, uint32_t size);
// ����������� ��� ������ � ���������� � ����� QUAD SPI
ALT_STATUS_CODE qspi_set_quad_mode(void);
// ���������� ����� ������ QUAD SPI
ALT_STATUS_CODE qspi_set_read_mode(void);
// ���������� ������� ���� � QSPI � 100��� (�� ��������� 50���)
ALT_STATUS_CODE qspi_set_100MHz(void);

int32_t wait_qspi_is_ready_timeout(uint32_t timeout_mks);

/**
 * ������ ������ QSPI � ������ DMA.
 * ������ ������ ���� ��������� �� 4� ����.
 * @param dma_buf - ����� ���� ������ �����������;
 * @param flash_addr - ����� ����������� ������;
 * @param size - ������ ����������� ������;
 * @return ������ ����������.
 */
ALT_STATUS_CODE qspi_dma_start_read(void *dma_buf, uint32_t flash_addr, uint32_t size);

/**
 * ������ ������ QSPI � ������ DMA.
 * ������ ������ ���� ��������� �� 4� ����.
 * @param dma_buf - ����� ������ ������ �����������;
 * @param flash_addr - ����� ������������ ������;
 * @param size - ������ ������������ ������;
 * @return ������ ����������.
 */
ALT_STATUS_CODE qspi_dma_start_write(void *dma_buf, uint32_t flash_addr, uint32_t size);

/**
 * Pooling DMA QSPI.
 * @return 1 - dma �����, 0 - dma ��������.
 */
char is_qspi_dma_busy(void);

#endif
