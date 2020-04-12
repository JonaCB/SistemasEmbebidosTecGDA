#include "uc_interrupt.h"


void void_func(void);
void void_func(){return;}
void void_func_uart(char chr);
void void_func_uart(char chr){(void)chr;return;}

void (*tim2_isr_func)(void) = void_func;
void (*tim3_isr_func)(void) = void_func;
void (*tim4_isr_func)(void) = void_func;

void (*uart_isr_func)(char) = void_func_uart;


void uc_interrupt_attatch_function(ISRType isr, void (*func)(void)){
    switch (isr)
    {
    case ISR_TIM2:
        tim2_isr_func = func;
        break;
    case ISR_TIM3:
        tim3_isr_func = func;
        break;
    case ISR_TIM4:
        tim4_isr_func = func;
        break;
    
    default:
        break;
    } 
}

void uc_interrupt_attatch_uart_function(void (*func)(char)){
 
        uart_isr_func = func;

}

void uc_interrupt_detach_function(ISRType isr){
    switch (isr)
    {
    case ISR_TIM2:
        tim2_isr_func = void_func;
        break;
    case ISR_TIM3:
        tim3_isr_func = void_func;
        break;
    case ISR_TIM4:
        tim4_isr_func = void_func;
        break;
    
    default:
        break;
    } 
}

void tim2_isr (void){

	timer_clear_flag(TIM2, TIM_SR_UIF);
	(*tim2_isr_func)();	
}

void tim3_isr (void){

	timer_clear_flag(TIM3, TIM_SR_UIF);
	(*tim3_isr_func)();	
}

void tim4_isr (void){

	timer_clear_flag(TIM4, TIM_SR_UIF);
	(*tim4_isr_func)();	
}

void usart1_isr (void){

	if (usart_get_flag (USART1, USART_SR_RXNE )){ // Recieve flag
		char chr = usart_recv(USART1);
        (*uart_isr_func)(chr);	
    }
}
