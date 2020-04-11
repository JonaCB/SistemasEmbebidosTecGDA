#ifndef UC_TIMER_H_   /* Include guard */
#define UC_TIMER_H_


#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>


void uc_timer_pwm_pin_setup(enum rcc_periph_clken gpio_clk, uint32_t gpio_port, uint16_t gpio_pin);
void uc_timer_setup(enum rcc_periph_clken timer_clk, uint32_t timer, uint32_t prescaler);
void uc_timer_pwm_setup(enum rcc_periph_clken timer_clk, uint32_t timer, enum tim_oc_id channel, uint32_t prescaler);
void uc_timer_config_period(uint32_t timer, uint32_t period);
void uc_timer_pwm_config_duty_cycle(uint32_t timer, enum tim_oc_id channel, uint32_t duty_cycle);
void uc_timer_enable_interrupt(uint32_t timer, uint8_t irqn);
void uc_timer_start(uint32_t timer);
void uc_timer_stop(uint32_t timer);



#endif // UC_TIMER_H_