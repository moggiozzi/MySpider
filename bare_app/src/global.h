#ifndef GLOBAL_H_
#define GLOBAL_H_

#ifndef SPI_DEFS_H_
#include "spi_defs.h"
#endif

#include <stdbool.h>

enum{
  ERR_OK = 0,
  ERR_MEMCPY = 1,
  ERR_QSPI_FLASH,
  ERR_MEMCMP,
  ERR_UST_CRC,
  ERR_CONF_CRC,
  ERR_NULL_PTR,
  ERR_QSPI_TIMEOUT,
  ERR_VER_CFG
};

#endif
