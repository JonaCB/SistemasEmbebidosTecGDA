/// @file uc_uart.c
//  Copyright 2020 Copyright Equipo 2
#include "uc_uart.h"
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "../miniprintf/miniprintf.h"

/**
 * Starts UART transmission.
 * @param[in] character
 */
void uart_putc(char ch)  {
    usart_send_blocking(USART1, ch);
}

/**
 * Prints UART message
 * param[in] format
 * param[out] rc
 */
int uart_printf(const char *format, ...)  {
    va_list args;
    int rc;

    va_start(args, format);
    rc = mini_vprintf_cooked(uart_putc, format, args);
    va_end(args);
    return rc;
}

/**
 * Sets up the UART peripheral pin ports needed.
 */
void uart_pin_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);
    // PA9 y PA10
    gpio_set_mode(GPIOA,
        GPIO_MODE_OUTPUT_50_MHZ,
        GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
        GPIO_USART1_TX);
    gpio_set_mode(GPIOA,  GPIO_MODE_INPUT,
        GPIO_CNF_INPUT_FLOAT,
        GPIO_USART1_RX);
}

/**
 * Sets up the UART configuration.
 */
void uart_setup(void) {
    rcc_periph_clock_enable(RCC_USART1);
    usart_set_baudrate(USART1, 115200);
    usart_set_databits(USART1, 8);
    usart_set_stopbits(USART1, USART_STOPBITS_1);
    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_parity(USART1, USART_PARITY_NONE);
    usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
}

/**
 * Enables UART receiving interrupt.
 */
void uart_enable_rx_interrupt(void) {
    usart_enable_rx_interrupt(USART1);
    nvic_clear_pending_irq(NVIC_USART1_IRQ);
    nvic_enable_irq(NVIC_USART1_IRQ);
}

/**
 * Starts UART.
 */
void uart_start(void) {
    usart_enable(USART1);
    usart_wait_send_ready(USART1);
}
