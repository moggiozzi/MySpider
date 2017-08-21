#include "timer.h"
#include <alt_timers.h>

ALT_STATUS_CODE init_timer(void) {
  // Значение таймера изменяется от максимума до 0
  // ALT_GPT_CPU_PRIVATE_TMR свой у каждого ядра
  // uint freq = alt_gpt_freq_get(ALT_GPT_CPU_PRIVATE_TMR); // частота таймера HZ
  ALT_STATUS_CODE st;
  uint32_t md = alt_gpt_maxcounter_get(ALT_GPT_CPU_PRIVATE_TMR);
  st = alt_gpt_counter_set(ALT_GPT_CPU_PRIVATE_TMR, md); // установить максимум
  if (st == ALT_E_SUCCESS)
    st = alt_gpt_mode_set(
      ALT_GPT_CPU_PRIVATE_TMR, ALT_GPT_RESTART_MODE_PERIODIC); // тип: одноразовый или периодический
  if (st == ALT_E_SUCCESS)
    st = alt_gpt_prescaler_set(ALT_GPT_CPU_PRIVATE_TMR, 0); // коэффициент таймера
//	if ( st == ALT_E_SUCCESS )
//		st = alt_gpt_int_enable(ALT_GPT_CPU_PRIVATE_TMR); // используется для вызова функции по окончанию таймера
  if (st == ALT_E_SUCCESS)
    st = alt_gpt_tmr_start(ALT_GPT_CPU_PRIVATE_TMR);
  return st;
}

uint32_t get_timers_diff(uint32_t t1, uint32_t t2) {
  if (t1 >= t2)
    return t1 - t2;
  else
    return 0xffffffff - t2 + t1;
}

// частота источника тактирования ALT_GPT_CPU_PRIVATE_TMR равна 200MHz. Можно прочитать функцией: alt_clk_freq_get(ALT_CLK_MPU_PERIPH, &freq);
// ALT_GPT_CPU_PRIVATE_TMR переполняется ~ через 21.4 сек
#define CPU_PRIVATE_TMR_FREQUENCY 200000000u
// тиков за 1 мкс
#define CPU_PRIVATE_TMR_TICKS_PER_US    (CPU_PRIVATE_TMR_FREQUENCY / 1000000u)
// максимальное время ожидания 20 сек в тиках
#define CPU_PRIVATE_TMR_MAX_CONTINUOUS_TICKS 4000000000u
// максимальное время ожидания 20 сек в мкс
#define CPU_PRIVATE_TMR_MAX_CONTINUOUS_US    (CPU_PRIVATE_TMR_MAX_CONTINUOUS_TICKS / CPU_PRIVATE_TMR_TICKS_PER_US)

void delay_mks(uint32_t mks) {
  uint32_t t1, t2;
  while (mks > CPU_PRIVATE_TMR_MAX_CONTINUOUS_US) // если время ожидания больше времени переполнения таймера
  {
    t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
    while (1) {
      t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
      if (get_timers_diff(t1, t2) >= CPU_PRIVATE_TMR_MAX_CONTINUOUS_TICKS) {
        mks -= CPU_PRIVATE_TMR_MAX_CONTINUOUS_US;
        break;
      }
    }
  }
  t1 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
  while (1) {
    t2 = alt_gpt_counter_get(ALT_GPT_CPU_PRIVATE_TMR);
    if (get_timers_diff(t1, t2) >= mks * CPU_PRIVATE_TMR_TICKS_PER_US)
      break;
  }
}
