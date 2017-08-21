#ifndef TIMER_H_
#define TIMER_H_

#ifndef __HWLIB_H__
#include <hwlib.h>
#endif
#ifndef __ALT_GPT_H__
#include "alt_timers.h"
#endif

ALT_STATUS_CODE init_timer(void);

// Только для таймеров у которых максимум равен 0xffffffff
uint32_t get_timers_diff(uint32_t t1, uint32_t t2);

void delay_mks(uint32_t mks);

#endif
