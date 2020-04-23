#include "delay.h"
#include "../system_common/system_common.h"
#include <libopencm3/stm32/timer.h> 
#include <libopencm3/stm32/rcc.h> 

void delay_setup(void) {

	timer_reset(TIM4);
	rcc_periph_clock_enable(RCC_TIM4);
	timer_set_prescaler(TIM4, F_CLK / 1000);
}

void delay_ms(uint32_t ms){

	timer_set_period(TIM4, ms);
	timer_enable_counter(TIM4);
	while (!timer_get_flag(TIM4, TIM_SR_UIF)) {}; //wait for the counter
	timer_clear_flag(TIM4, TIM_SR_UIF);
	timer_disable_counter(TIM4);
}