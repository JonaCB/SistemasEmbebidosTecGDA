

#ifndef DIMMER_PARAMS_H_   /* Include guard */
#define DIMMER_PARAMS_H_


#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "../system_common/system_common.h"

#define PWM_PRESCALER          1
#define PWM_PIN_PORT_CLOCK     RCC_GPIOA
#define PWM_PIN_PORT           GPIOA
#define PWM_PIN                GPIO1
#define PWM_TIMER_CLOCK        RCC_TIM2
#define PWM_TIMER              TIM2
#define PWM_TIMER_CHANNEL      TIM_OC2
#define PWM_FREQUENCY 10000 //PWM Frequency = 10kHz
#define PWM_PERIOD F_CLK/PWM_PRESCALER/PWM_FREQUENCY //2400 counts,  couter 100 - 65000


#endif // DIMMER_PARAMS_H_