
#ifndef UC_UART_H_   /* Include guard */
#define UC_UART_H_

void usart_setup(void);
void uart_putc( char ch);
int uart_printf(const char *format,...);


#endif // UC_UART_H_