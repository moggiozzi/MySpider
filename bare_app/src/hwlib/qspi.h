#ifndef QSPI_H_
#define QSPI_H_

#ifndef __HWLIB_H__
#include "hwlib.h"
#endif

#include <alt_dma_common.h>
#include <alt_dma.h>

#define WAIT_QSPI_TIMEOUT_MKS (10*1000000) // 10 секунд

ALT_STATUS_CODE init_qspi(void);

uint32_t qspi_is_ready(void);
// Прочитать регистры чипа памяти и контроллера qspi (Только для отладки)
void qspi_read_regs(void);
// Стереть size памяти по адресу addr (!!! выравнивание до сектора в 64Кб !!!)
ALT_STATUS_CODE qspi_clear(uint32_t addr, uint32_t size);
// Переключить чип памяти и контроллер в режим QUAD SPI
ALT_STATUS_CODE qspi_set_quad_mode(void);
// Установить режим чтения QUAD SPI
ALT_STATUS_CODE qspi_set_read_mode(void);
// Установить частоты чипа и QSPI в 100МГц (по умолчанию 50МГц)
ALT_STATUS_CODE qspi_set_100MHz(void);

int32_t wait_qspi_is_ready_timeout(uint32_t timeout_mks);

/**
 * Чтение данных QSPI в режиме DMA.
 * Данные должны быть выровнены до 4х байт.
 * @param dma_buf - буфер куда данные считываются;
 * @param flash_addr - адрес считываемых данных;
 * @param size - размер считываемых данных;
 * @return статус выполнения.
 */
ALT_STATUS_CODE qspi_dma_start_read(void *dma_buf, uint32_t flash_addr, uint32_t size);

/**
 * Запись данных QSPI в режиме DMA.
 * Данные должны быть выровнены до 4х байт.
 * @param dma_buf - буфер откуда данные записываюся;
 * @param flash_addr - адрес записываемых данных;
 * @param size - размер записываемых данных;
 * @return статус выполнения.
 */
ALT_STATUS_CODE qspi_dma_start_write(void *dma_buf, uint32_t flash_addr, uint32_t size);

/**
 * Pooling DMA QSPI.
 * @return 1 - dma занят, 0 - dma свободен.
 */
char is_qspi_dma_busy(void);

#endif
