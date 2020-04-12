#ifndef UC_TIMER_H_   /* Include guard */
#define UC_TIMER_H_


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>

void uc_timer_setup(uint32_t timer, enum rcc_periph_clken clken, uint32_t prescaler);
void uc_timer_set_period(uint32_t timer, uint32_t period);
void uc_timer_enable_interrupt(uint32_t timer, uint8_t irqn);
void uc_timer_pwm_setup(uint32_t timer, enum rcc_periph_clken clken, uint32_t prescaler, enum tim_oc_id channel);
void uc_timer_start(uint32_t timer);
void uc_timer_pin_setup(enum rcc_periph_clken clken, uint32_t gpioport, uint16_t pin);
void uc_timer_set_duty_cycle(uint32_t timer, enum tim_oc_id channel, uint32_t duty_cycle);

#endif // UC_TIMER_H_



	// uc_timer_pin_setup(RCC_GPIOA, GPIOA, GPIO1);

	// uc_timer_pwm_setup(TIM2, RCC_TIM2, PRESCALER_TIM2, TIM_OC2);
	// uc_timer_set_period(TIM2, PWM_PERIOD);
	// uc_timer_set_duty_cycle(TIM2, TIM_OC2, get_pwm_percentage_counts(dimPercentage));
	// uc_timer_start(TIM2);


    	// uc_timer_setup(TIM3, RCC_TIM3, PRESCALER_TIM3);
	// uc_timer_set_period(TIM3, PRESCALER_TIM3);

	// uc_timer_enable_interrupt(TIM3, NVIC_TIM3_IRQ);

	// uc_timer_start(TIM3);