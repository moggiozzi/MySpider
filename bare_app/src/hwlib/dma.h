#ifndef DMA_H_
#define DMA_H_

#include <hwlib.h>

ALT_STATUS_CODE init_dma(void);

/**
 * Pooling DMA.
 *
 * @param dma_channel - выделенный DMA канал;
 * @return статус выполнения.
 */
ALT_STATUS_CODE get_dma_state(ALT_DMA_CHANNEL_t dma_channel);

#endif
