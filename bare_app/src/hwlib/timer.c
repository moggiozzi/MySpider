#include "timer.h"
#include <alt_timers.h>

ALT_STATUS_CODE init_timer(void) {
  // �������� ������� ���������� �� ��������� �� 0
  // ALT_GPT_CPU_PRIVATE_TMR ���� � ������� ����
  // uint freq = alt_gpt_freq_get(ALT_GPT_CPU_PRIVATE_TMR); // ������� ������� HZ
  ALT_STATUS_CODE st;
  uint32_t md = alt_gpt_maxcounter_get(ALT_GPT_CPU_PRIVATE_TMR);
  st = alt_gpt_counter_set(ALT_GPT_CPU_PRIVATE_TMR, md); // ���������� ��������
  if (st == ALT_E_SUCCESS)
    st = alt_gpt_mode_set(
      ALT_GPT_CPU_PRIVATE_TMR, ALT_GPT_RESTART_MODE_PERIODIC); // ���: ����������� ��� �������������
  if (st == ALT_E_SUCCESS)
    st = alt_gpt_prescaler_set(ALT_GPT_CPU_PRIVATE_TMR, 0); // ����������� �������
//	if ( st == ALT_E_SUCCESS )
//		st = alt_gpt_int_enable(ALT_GPT_CPU_PRIVATE_TMR); // ������������ ��� ������ ������� �� ��������� �������
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

// ������� ��������� ������������ ALT_GPT_CPU_PRIVATE_TMR ����� 200MHz. ����� ��������� ��������: alt_clk_freq_get(ALT_CLK_MPU_PERIPH, &freq);
// ALT_GPT_CPU_PRIVATE_TMR ������������� ~ ����� 21.4 ���
#define CPU_PRIVATE_TMR_FREQUENCY 200000000u
// ����� �� 1 ���
#define CPU_PRIVATE_TMR_TICKS_PER_US    (CPU_PRIVATE_TMR_FREQUENCY / 1000000u)
// ������������ ����� �������� 20 ��� � �����
#define CPU_PRIVATE_TMR_MAX_CONTINUOUS_TICKS 4000000000u
// ������������ ����� �������� 20 ��� � ���
#define CPU_PRIVATE_TMR_MAX_CONTINUOUS_US    (CPU_PRIVATE_TMR_MAX_CONTINUOUS_TICKS / CPU_PRIVATE_TMR_TICKS_PER_US)

void delay_mks(uint32_t mks) {
  uint32_t t1, t2;
  while (mks > CPU_PRIVATE_TMR_MAX_CONTINUOUS_US) // ���� ����� �������� ������ ������� ������������ �������
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
