#include "diskio.h"
#include "diskio_mmc.h"
#include "alt_sdmmc.h"

#define SDMMC_BLOCK_SZ  512
#define SDMMC_CHUNK_SZ  (512 * 1024) // This is the largest chunk read before handling the watchdog.

ALT_SDMMC_CARD_INFO_t card_info;

int MMC_disk_status(void)
{
	return 0;
}

int MMC_disk_initialize(void)
{
  ALT_STATUS_CODE status = ALT_E_SUCCESS;
  if (status == ALT_E_SUCCESS)
  {
      status = alt_sdmmc_init();
  }
  if (status == ALT_E_SUCCESS)
  {
      status = alt_sdmmc_card_pwr_on();
  }
  if (status == ALT_E_SUCCESS)
  {
      status = alt_sdmmc_card_identify(&card_info);
  }
  if (status == ALT_E_SUCCESS)
  {
      status = alt_sdmmc_card_bus_width_set(&card_info, ALT_SDMMC_BUS_WIDTH_4);
  }
  if (status == ALT_E_SUCCESS)
  {
      status = alt_sdmmc_fifo_param_set((ALT_SDMMC_FIFO_NUM_ENTRIES >> 3) - 1,
                                        (ALT_SDMMC_FIFO_NUM_ENTRIES >> 3), (ALT_SDMMC_MULT_TRANS_t)0);
  }
  if (status == ALT_E_SUCCESS)
  {
      //status = alt_sdmmc_dma_enable();
  }
  if (status == ALT_E_SUCCESS)
  {
      uint32_t speed = card_info.xfer_speed;
      if (card_info.high_speed)
      {
          speed *= 2;
      }
      status = alt_sdmmc_card_speed_set(&card_info, speed);
  }
  //MPL_WATCHDOG();
  if (status == ALT_E_SUCCESS)
  {
      return 0;
  }
  else
  {
      return STA_NOINIT;
  }
}

int MMC_disk_read(
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
  ALT_STATUS_CODE status = ALT_E_SUCCESS;
  status = alt_sdmmc_read(&card_info, buff, (void *)(sector * SDMMC_BLOCK_SZ), (count * SDMMC_BLOCK_SZ));
  if (status != ALT_E_SUCCESS)
  {
      return RES_ERROR;
  }
  return RES_OK;
}

int MMC_disk_write(
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
  ALT_STATUS_CODE status = ALT_E_SUCCESS;
  status = alt_sdmmc_write(&card_info, (void *)(sector * SDMMC_BLOCK_SZ), (void *)buff, (count * SDMMC_BLOCK_SZ));
  if (status != ALT_E_SUCCESS)
  {
      return RES_ERROR;
  }
  return RES_OK;
}
