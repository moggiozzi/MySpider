#include "alt_dma.h"
#include "dma.h"

ALT_STATUS_CODE init_dma(void)
{
  ALT_STATUS_CODE status = ALT_E_SUCCESS;
  ALT_DMA_CFG_t dma_config;

  dma_config.manager_sec = ALT_DMA_SECURITY_DEFAULT;
  for (int i = 0; i < 8; ++i)
    dma_config.irq_sec[i] = ALT_DMA_SECURITY_DEFAULT;
  for (int i = 0; i < 32; ++i)
    dma_config.periph_sec[i] = ALT_DMA_SECURITY_DEFAULT;
  for (int i = 0; i < 4; ++i)
    dma_config.periph_mux[i] = ALT_DMA_PERIPH_MUX_DEFAULT;

  status = alt_dma_init(&dma_config);
  return status;
}

/**
 * Pooling DMA.
 *
 * @param dma_channel - выделенный DMA канал;
 * @return статус выполнения.
 */
ALT_STATUS_CODE get_dma_state(ALT_DMA_CHANNEL_t dma_channel)
{
  ALT_DMA_CHANNEL_STATE_t channel_state;  // Статус выделенного канала.

  ALT_STATUS_CODE status = alt_dma_channel_state_get(dma_channel, &channel_state);

  if (status == ALT_E_SUCCESS)
  {
    status = ALT_E_ERROR;

    if (channel_state == ALT_DMA_CHANNEL_STATE_STOPPED)
    {
      status = ALT_E_SUCCESS;
    }
  }

  return status;
}
