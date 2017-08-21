#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>

typedef struct PWM_SHARED_DATA
{
	uint32_t gpio_idx;
	uint32_t pwm_period;
	uint32_t pwm_duty;
}PWM_SHARED_DATA;

typedef struct PWM_DATA
{
	uint32_t gpio_idx;
	uint32_t pwm_period;
	uint32_t pwm_duty;
	uint32_t pwm_last;
	uint32_t val;
}PWM_DATA;

void doPwm(void);

#endif
