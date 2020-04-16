//  Copyright 2020 Copyright Equipo 2
#ifndef DIMMER_PARAMS_H_   /* Include guard */
#define DIMMER_PARAMS_H_

#include <libopencm3/stm32/timer.h>  /// <Common timer macros and functions.
#include <libopencm3/stm32/rcc.h>    /// <Common clock macros and functions.
#include <libopencm3/stm32/gpio.h>   /// <Common GPIO macros and functions.
#include "../system_common/system_common.h"  /// <Common system macros and
                                             /// functions.

#define PWM_PRESCALER          1  /// <PWM prescaler constant define
#define PWM_PIN_PORT_CLOCK     RCC_GPIOA  /// <Port clock address for the
                                          /// PWM define
#define PWM_PIN_PORT           GPIOA  /// <GPIO module address for the
                                      /// PWM define
#define PWM_PIN                GPIO1  /// <PIN for the PWM define
#define PWM_TIMER_CLOCK        RCC_TIM2  /// <Timer clock address for the
                                         /// PWM module
#define PWM_TIMER              TIM2  /// <Timer address for the PWM module
#define PWM_TIMER_CHANNEL      TIM_OC2  /// <Timer output compare
                                        /// channel address
#define PWM_FREQUENCY          10000  /// <PWM Frequency = 10kHz
#define PWM_PERIOD F_CLK/PWM_PRESCALER/PWM_FREQUENCY  /// <2400 counts,
                                                      /// counter 100 - 65000

#endif  // ABSTRACTION_AND_DOCUMENTATION_DIMMER_DIMMER_PARAMS_H_
