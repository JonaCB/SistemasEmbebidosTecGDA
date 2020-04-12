
#include "uc_uart.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include "../miniprintf/miniprintf.h"

 void usart_setup(void){

	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_USART1);

	// UART TX on PA9 (GPIO_USART1_TX)
	gpio_set_mode(GPIOA,
		GPIO_MODE_OUTPUT_50_MHZ,
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
		GPIO_USART1_TX);
	gpio_set_mode(GPIOA, GPIO_MODE_INPUT, 
		GPIO_CNF_INPUT_FLOAT, 
		GPIO_USART1_RX);

	usart_set_baudrate(USART1,115200);
	usart_set_databits(USART1,8);
	usart_set_stopbits(USART1,USART_STOPBITS_1);
	usart_set_mode(USART1,USART_MODE_TX_RX);
	usart_set_parity(USART1,USART_PARITY_NONE);
	usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);


	usart_enable_rx_interrupt (USART1);
	nvic_clear_pending_irq(NVIC_USART1_IRQ); 
	nvic_enable_irq(NVIC_USART1_IRQ);
	usart_enable(USART1);
	usart_wait_send_ready (USART1);
}

void uart_putc( char ch) {
	usart_send_blocking(USART1,ch);
}

int uart_printf(const char *format,...) {
	va_list args;
	int rc;

	va_start(args,format);
	rc = mini_vprintf_cooked(uart_putc,format,args);
	va_end(args);
	return rc;
}