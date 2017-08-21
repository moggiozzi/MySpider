#ifndef FPGA_H_
#define FPGA_H_

#ifndef __HWLIB_H__
#include "hwlib.h"
#endif

ALT_STATUS_CODE init_fpga(bool isWaitReady);

ALT_STATUS_CODE wait_fpga_ready(void);

ALT_STATUS_CODE restart_fpga(bool isWaitReady);

#endif
