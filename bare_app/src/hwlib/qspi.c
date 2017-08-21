#include "qspi.h"
#include "hwlib.h"
#include "socal/socal.h"
#include "socal/hps.h"
#include "socal/alt_qspi.h"
#include "alt_qspi.h"
#include "alt_qspi_private.h"
#include <alt_dma_common.h>
#include <alt_dma.h>
#include "dma.h"
#include "timer.h"

#define QSPI_DMA_SINGLE_SIZE  4
#define QSPI_DMA_BURST_SIZE   64

char is_qspi_dma_write = 0;  // Флаг индикации записи DMA.
char is_qspi_dma_read  = 0;  // Флаг индикации чтение DMA.

ALT_DMA_CHANNEL_t qspi_dma_channel = (ALT_DMA_CHANNEL_t) -1;  // Выделенный канал DMA.

ALT_STATUS_CODE init_qspi(void)
{
  ALT_STATUS_CODE status = ALT_E_SUCCESS;

  // Инициализация QSPI.
  if (status == ALT_E_SUCCESS)
  {
    status = alt_qspi_init();
  }

  //    if(status == ALT_E_SUCCESS)
  //    {
  //        printf("INFO: Configuring QSPI DMA single size = %d, burst_size = %d bytes.\n", QSPI_DMA_SINGLE_SIZE, QSPI_DMA_BURST_SIZE);
  //        status = alt_qspi_dma_config_set(QSPI_DMA_SINGLE_SIZE, QSPI_DMA_BURST_SIZE);
  //    }

  // Включение QSPI.
  if (status == ALT_E_SUCCESS)
  {
    status = alt_qspi_enable();
  }

  if (status == ALT_E_SUCCESS)
  {
    if (!alt_qspi_is_idle())
    {
      status = ALT_E_ERROR;
    }
  }

  // Конфигурирование DMA.
  if (status == ALT_E_SUCCESS)
  {
    status = alt_qspi_dma_config_set(QSPI_DMA_SINGLE_SIZE, QSPI_DMA_BURST_SIZE);
  }

  // Включение DMA.
  if (status == ALT_E_SUCCESS)
  {
    status = alt_qspi_dma_enable();
  }

  return status;
}

uint32_t qspi_is_ready(void){
  uint32_t status, sr, fsr;
  status = sr = fsr = 0;
  status = alt_qspi_stig_rd_cmd(ALT_QSPI_STIG_OPCODE_RDSR, 0, 1, &sr, 10000);
  status |= alt_qspi_stig_rd_cmd(ALT_QSPI_STIG_OPCODE_RDFLGSR, 0, 1, &fsr, 10000);
  return ALT_QSPI_STIG_SR_BUSY_GET(sr)==0 && ALT_QSPI_STIG_FLAGSR_PROGRAMREADY_GET(fsr)==1;
}

// Ожидание qspi с таймаутом в мкс
int32_t wait_qspi_is_ready_timeout(uint32_t timeout_mks)
{
  int32_t res;
  while(!qspi_is_ready() && timeout_mks--)
  {
    delay_mks(1);
  }
  if (timeout_mks)
    res = ALT_E_SUCCESS;
  else
    res = ALT_E_ERROR;
  return res;
}

void qspi_read_regs(void){
  volatile uint32_t tmp;
  uint32_t state,cfg,devrd,devwr,indwr,indwrstaddr,flashcmd;
  uint32_t sr,flagsr,vol_cfg_reg,nvol_cfg_reg,enh_cfg_reg;
  ALT_QSPI_DEV_INST_CONFIG_t write_cfg;
  ALT_QSPI_DEV_INST_CONFIG_t read_cfg;
  state = alt_qspi_device_write_config_get(&write_cfg);
  state |= alt_qspi_device_read_config_get(&read_cfg);

  cfg = alt_read_word(ALT_QSPI_CFG_ADDR);
  // If the idle field of the QSPI configuration register is 1 then the serial interface and QSPI pipeline is idle.
  tmp = ALT_QSPI_CFG_IDLE_GET(cfg);
  //enable
  tmp = ALT_QSPI_CFG_EN_GET(cfg);
  tmp = ALT_QSPI_CFG_BAUDDIV_GET(cfg);

  devrd = alt_read_word(ALT_QSPI_DEVRD_ADDR);
  tmp = ALT_QSPI_DEVRD_RDOPCODE_GET(devrd);
  tmp = ALT_QSPI_DEVRD_INSTWIDTH_GET(devrd);
  tmp = ALT_QSPI_DEVRD_ADDRWIDTH_GET(devrd);
  tmp = ALT_QSPI_DEVRD_DATAWIDTH_GET(devrd);
  tmp = ALT_QSPI_DEVRD_DUMMYRDCLKS_GET(devrd);

  devwr = alt_read_word(ALT_QSPI_DEVWR_ADDR);
  tmp = ALT_QSPI_DEVWR_WROPCODE_GET(devwr);
  tmp = ALT_QSPI_DEVWR_ADDRWIDTH_GET(devwr);
  tmp = ALT_QSPI_DEVWR_DATAWIDTH_GET(devwr);
  tmp = ALT_QSPI_DEVWR_DUMMYWRCLKS_GET(devwr);

  state |= alt_qspi_stig_rd_cmd(ALT_QSPI_STIG_OPCODE_RDFLGSR, 0, 1, &flagsr, 10000);
  state |= alt_qspi_stig_rd_cmd(ALT_QSPI_STIG_OPCODE_RDSR, 0, 1, &sr, 10000);
  state |= alt_qspi_stig_rd_cmd(0x85, 0, 1, &vol_cfg_reg, 10000); // READ VOLATILE CONFIGURATION REGISTER
  state |= alt_qspi_stig_rd_cmd(0xb5, 0, 2, &nvol_cfg_reg, 10000); // READ NONVOLATILE CONFIGURATION REGISTER
  state |= alt_qspi_stig_rd_cmd(0x65, 0, 1, &enh_cfg_reg, 10000); // READ ENHANCED VOLATILE CONFIGURATION REGISTER

  indwr = alt_read_word(ALT_QSPI_INDWR_ADDR);
  //This field is set to 1 when an indirect operation has completed. Write a 1 to this field to clear it.
  // 1 - in progress
  tmp = ALT_QSPI_INDWR_RDSTAT_GET(indwr);
  tmp = ALT_QSPI_INDWR_INDDONE_GET(indwr);

  indwrstaddr = alt_read_word(ALT_QSPI_INDWRSTADDR_ADDR);
  tmp = ALT_QSPI_INDWRSTADDR_ADDR_GET(indwrstaddr);

  flashcmd = alt_read_word(ALT_QSPI_FLSHCMD_ADDR);
  tmp = ALT_QSPI_FLSHCMD_CMDOPCODE_GET(flashcmd);
  return;
}

ALT_STATUS_CODE qspi_clear(uint32_t addr, uint32_t size){
  uint32_t state=ALT_E_SUCCESS;
// todo
// v16.1 нет функции alt_qspi_erase_subsector (была в v15.0)
//  if ( size <= 3 * ALT_QSPI_SUBSECTOR_SIZE ) // 4Кб стирается за 231мс, 64Кб - за 722мс
//  {
//    for(int i = 0; i < size; i+= ALT_QSPI_SUBSECTOR_SIZE) // 4 KiB
//      if (state == ALT_E_SUCCESS)
//      {
//        state |= alt_qspi_erase_subsector(addr + i);
//        while(!qspi_is_ready());
//      }
//  }
//  else
//  {
    for(int i = 0; i < size; i+= ALT_QSPI_SECTOR_SIZE) // 64 KiB
      if (state == ALT_E_SUCCESS)
      {
        state |= alt_qspi_erase_sector(addr + i);
        state |= wait_qspi_is_ready_timeout(WAIT_QSPI_TIMEOUT_MKS);
      }
//  }
  return state;
}

ALT_STATUS_CODE qspi_set_quad_mode(void){
  uint32_t enh_cfg_reg;
  ALT_QSPI_DEV_INST_CONFIG_t write_cfg;
  ALT_QSPI_DEV_INST_CONFIG_t read_cfg;
  ALT_STATUS_CODE state = alt_qspi_device_write_config_get(&write_cfg);
  state |= alt_qspi_device_read_config_get(&read_cfg);
  if (alt_qspi_is_idle())
  {
    write_cfg.addr_xfer_type = ALT_QSPI_MODE_QUAD;
    write_cfg.data_xfer_type = ALT_QSPI_MODE_QUAD;
    write_cfg.inst_type = ALT_QSPI_MODE_QUAD;
    //write_cfg.dummy_cycles;
    write_cfg.op_code = ALT_QSPI_STIG_OPCODE_QUAD_PP;

    // изменить настройки чипа flash памяти
    state |= alt_qspi_stig_rd_cmd(0x65, 0, 1, &enh_cfg_reg, 10000); // READ ENHANCED VOLATILE CONFIGURATION REGISTER
    enh_cfg_reg &= 0x7A; // 7й бит 0-QUAD enable 1-QUAD disable; 0:2 биты - импеданс 010-60ОМ
    state |=alt_qspi_device_wren();
    state |=alt_qspi_stig_wr_cmd(0x61,0,1,&enh_cfg_reg,10000); // WRITE ENHANCED VOLATILE CONFIGURATION REGISTER

    // изменить настройки qspi контроллера
    state |= alt_qspi_device_write_config_set(&write_cfg);

    state |= alt_qspi_device_write_config_get(&write_cfg);
    state |= alt_qspi_device_read_config_get(&read_cfg);

    state |=alt_qspi_device_wrdis();
    state |= alt_qspi_stig_rd_cmd(0x65, 0, 1, &enh_cfg_reg, 10000); // READ ENHANCED VOLATILE CONFIGURATION REGISTER

    if (enh_cfg_reg & 0x80)
      state = ALT_E_ERROR;
  }
  return state;
}

// todo: Не работает
ALT_STATUS_CODE qspi_set_read_mode(void){
  uint32_t state = 0;
  ALT_QSPI_DEV_INST_CONFIG_t read_cfg;
  state |= alt_qspi_device_read_config_get(&read_cfg);

//  // 1MB за 216мс
//  read_cfg.addr_xfer_type = ALT_QSPI_MODE_SINGLE;
//  read_cfg.data_xfer_type = ALT_QSPI_MODE_SINGLE;
//  read_cfg.inst_type      = ALT_QSPI_MODE_SINGLE;
//  read_cfg.dummy_cycles   = 0;
//  read_cfg.op_code        = 0x03;

  // По умолчанию 1MB за 124мс
  read_cfg.addr_xfer_type = ALT_QSPI_MODE_QUAD;
  read_cfg.data_xfer_type = ALT_QSPI_MODE_QUAD;
  read_cfg.inst_type      = ALT_QSPI_MODE_SINGLE;
  read_cfg.dummy_cycles   = 10;
  read_cfg.op_code        = 0xEB;

//  // 1MB за 135 мс
//  read_cfg.addr_xfer_type = ALT_QSPI_MODE_DUAL;
//  read_cfg.data_xfer_type = ALT_QSPI_MODE_DUAL;
//  read_cfg.inst_type      = ALT_QSPI_MODE_SINGLE;
//  read_cfg.dummy_cycles   = 8;
//  read_cfg.op_code        = 0xbb;

  state |= alt_qspi_device_read_config_set(&read_cfg);

  return state;
}

ALT_STATUS_CODE qspi_set_100MHz(void){
  ALT_QSPI_TIMING_CONFIG_t tcfg;
  int32_t state = ALT_E_SUCCESS;
  state |= alt_qspi_timing_config_get(&tcfg);
  tcfg.rd_datacap = 4; // Delay the read data capturing logic by the programmed number of qspi_clk cycles
  state |= alt_qspi_timing_config_set(&tcfg);
  state |= alt_qspi_baud_rate_div_set(ALT_QSPI_BAUD_DIV_4); // ALT_QSPI_BAUD_DIV_4 для 100 MHz
  return state;
}

/**
 * Запись данных QSPI в режиме DMA.
 * Данные должны быть выровнены до 4х байт.
 *
 * @param dma_buf - буфер откуда данные записываюся;
 * @param flash_addr - адрес записываемых данных;
 * @param size - размер записываемых данных;
 * @return статус выполнения.
 */
ALT_STATUS_CODE qspi_dma_start_write(void *dma_buf, uint32_t flash_addr, uint32_t size)
{
  ALT_STATUS_CODE status = ALT_E_SUCCESS;

  // Start the indirect read QSPI.
  if (status == ALT_E_SUCCESS)
  {
    status = alt_qspi_indirect_write_start(flash_addr, size);
  }

  if (status == ALT_E_SUCCESS)
  {
    // QSPI.
    status = alt_qspi_indirect_read_finish();

    // Выделение свободного DMA канала для использования, если такой имеется.
    if (status == ALT_E_SUCCESS)
    {
      status = alt_dma_channel_alloc_any(&qspi_dma_channel);
    }

    // Start the DMA transfer.
    ALT_DMA_PROGRAM_t dma_program;

    status = alt_dma_memory_to_periph(qspi_dma_channel, &dma_program, ALT_DMA_PERIPH_QSPI_FLASH_TX, dma_buf, size, NULL, false, ALT_DMA_EVENT_0);

    // Выставление флага записи DMA.
    is_qspi_dma_write = 1;
  }
  else
  {
    // QSPI.
    status = alt_qspi_indirect_read_cancel();
  }

  return status;
}

/**
 * Чтение данных QSPI в режиме DMA.
 * Данные должны быть выровнены до 4х байт.
 *
 * @param dma_buf - буфер куда данные считываются;
 * @param flash_addr - адрес считываемых данных;
 * @param size - размер считываемых данных;
 * @return статус выполнения.
 */
ALT_STATUS_CODE qspi_dma_start_read(void *dma_buf, uint32_t flash_addr, uint32_t size)
{
	ALT_STATUS_CODE status = ALT_E_SUCCESS;

	// Start the indirect read QSPI.
	if (status == ALT_E_SUCCESS)
	{
    status = alt_qspi_indirect_read_start(flash_addr, size);  // Перенос данных из flash в sdram.
	}

	// ATTENTION!!! Процесс должен ожидать завершения транзакции SPI.

	if (status == ALT_E_SUCCESS)
	{
    // QSPI.
    status = alt_qspi_indirect_read_finish();

    if (status == ALT_E_SUCCESS)
    {
      status = alt_dma_channel_alloc_any(&qspi_dma_channel);
    }

    // Start the DMA transfer.
    ALT_DMA_PROGRAM_t dma_program;

		status = alt_dma_periph_to_memory(qspi_dma_channel, &dma_program, dma_buf, ALT_DMA_PERIPH_QSPI_FLASH_RX, size, NULL, false, ALT_DMA_EVENT_0);

    // Выставление флага чтения DMA.
    is_qspi_dma_read = 1;
	}
	else
	{
	  // QSPI.
	  status = alt_qspi_indirect_read_cancel();
	}

	return status;
}

/**
 * Pooling DMA QSPI.
 *
 * @return 1 - dma занят, 0 - dma свободен.
 */
char is_qspi_dma_busy(void)
{
  char busy = 0;  // DMA свободен.

  if (is_qspi_dma_write || is_qspi_dma_read)
  {
    busy = 1;  // DMA занят.

    ALT_STATUS_CODE status = get_dma_state(qspi_dma_channel);

    // Транзакция закончилась успешно или неудачно.
    if ( (status == ALT_E_SUCCESS) || (status == ALT_E_ERROR) )
    {
      is_qspi_dma_write = 0;
      is_qspi_dma_read = 0;

      // Освобождение канала DMA.
      alt_dma_channel_free(qspi_dma_channel);

      busy = 0;  // DMA свободен.
    }
  }

  return busy;
}
