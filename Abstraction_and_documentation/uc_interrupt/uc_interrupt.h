//  Copyright 2020 Copyright Equipo 2
#ifndef UC_INTERRUPT_H_   /* Include guard */
#define UC_INTERRUPT_H_


#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/usart.h>

typedef enum {
  ISR_TIM2,
  ISR_TIM3,
  ISR_TIM4
} ISRType;


void uc_interrupt_attatch_function(ISRType isr, void (*func)(void));
void uc_interrupt_detach_function(ISRType isr);
void uc_interrupt_attatch_uart_function(void (*func)(char));


#endif  // ABSTRACTION_AND_DOCUMENTATION_UC_INTERRUPT_UC_INTERRUPT_H_
