#ifndef UC_UART_H_   /* Include guard */
#define UC_UART_H_



void uart_pin_setup(void);
void uart_setup(void);
void uart_enable_rx_interrupt(void);
void uart_start(void);
void uart_putc( char ch);
int uart_printf(const char *format,...);




#endif // UC_UART_H_