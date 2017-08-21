#include "pwm.h"
#include <alt_generalpurpose_io.h>
#include "pmu.h"

void setPwm(PWM_DATA * p, uint32_t gpioIdx, uint32_t period, uint32_t duty){
	p->gpio_idx = gpioIdx;
	p->pwm_period = period;
	p->pwm_duty = duty;
	p->val = 0;
}

#define N 20
#define mks2ticks(x) ((x)*800) // 800 MHz

uint32_t diff(uint32_t last, uint32_t current){
	if(current > last)
		return current - last;
	return 0xffffffff - last + current;
}

void doPwm(void)
{
  uint32_t max_period_err = 0;
  uint32_t max_duty_err = 0;

  PWM_DATA pwm[N];
  for(int i=0;i<N;i++){
	  setPwm(&pwm[i],i,mks2ticks(20000),mks2ticks(1000+i*1000));
	  alt_gpio_port_datadir_set(ALT_GPIO_PORTA, 1 << i, 1); // output
  }
  uint32_t cur_ticks;
  uint32_t td;
  bool isFirst = true;
  while(1){
	  for(int i=0;i<N;i++){
		  PWM_DATA * p = &pwm[i];
		  cur_ticks = get_pmu_cyclecount();
		  td = diff(p->pwm_last,cur_ticks);
		  if( td > p->pwm_period)
		  {
			  if(p->val == 0)
			  {
				  alt_gpio_port_data_write(ALT_GPIO_PORTA, 1 << p->gpio_idx, 1);//1
				  p->val = 1;
				  if(td > 2 * p->pwm_period) // опоздали больше чем на 2 периода
					  p->pwm_last = cur_ticks;
				  else
				  {
					  p->pwm_last += p->pwm_period; //следующий период
				  }
				  if (!isFirst && td - p->pwm_period > max_period_err)
					  max_period_err = td - p->pwm_period;
			  }
		  }else{
			  if(td > p->pwm_duty && p->val == 1){
				  alt_gpio_port_data_write(ALT_GPIO_PORTA, 1 << p->gpio_idx, 0);//0
				  p->val = 0;
				  if (!isFirst && td - p->pwm_duty > max_duty_err)
					  max_duty_err = td - p->pwm_duty;
			  }
		  }
	  }
	  isFirst = false;
  }
}
