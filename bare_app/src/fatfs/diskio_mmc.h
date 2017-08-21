#ifndef DISKIO_MMC_H_
#define DISKIO_MMC_H_

#include "integer.h"

int MMC_disk_status(void);

int MMC_disk_initialize(void);

int MMC_disk_read(
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
);

int MMC_disk_write(
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
);

#endif
