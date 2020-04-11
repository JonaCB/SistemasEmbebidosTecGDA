#ifndef RINGTONE_PARAMS_H_   /* Include guard */
#define RINGTONE_PARAMS_H_


#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include "../uc_interrupt/uc_interrupt.h"


#define RINGTONE_PRESCALER_PWM          12 
#define RINGTONE_PWM_PIN_PORT_CLOCK     RCC_GPIOB
#define RINGTONE_PWM_PIN_PORT           GPIOB
#define RINGTONE_PWM_PIN                GPIO7
#define RINGTONE_PWM_TIMER_CLOCK        RCC_TIM4
#define RINGTONE_PWM_TIMER              TIM4
#define RINGTONE_PWM_TIMER_CHANNEL      TIM_OC2

#define RINGTONE_TIMER_PRESCALER         2400 //TIM3 frequency = 64Mhz/64 = 1Mhz
#define RINGTONE_TIMER_CLOCK            RCC_TIM2
#define RINGTONE_TIMER                  TIM2
#define RINGTONE_TIMER_INTERRUPT        NVIC_TIM2_IRQ
#define RINGTONE_TIMER_INTERRUPT_ISR    ISR_TIM2



#endif // RINGTONE_PARAMS_H_