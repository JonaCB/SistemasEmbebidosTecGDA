
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#define F_CLK 24000000
#define PRESCALER_TIM3 F_CLK / 1000 //TIM3 frequency = 24Mhz/24K = 1khz
#define PERIOD_TIM3 500 //500ms

static void gpio_setup(void);
static void system_clock_setup(void);
static void TIM3_setup(void);
static void usart_setup(void);
static inline void uart_putc( char ch);
static void uart_send_msg( char *msg);

typedef enum {
  IDLE,
  SENDING_MSG,
  SENDING_CHAR,
} TxSMType; 

TxSMType tx_state = IDLE;

char * msg_addr;

int main(void) {

	system_clock_setup();
	gpio_setup();
	usart_setup();
	TIM3_setup();

	for (;;) {

	}

	return 0;
}

static void usart_setup(void){

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

static inline void uart_putc( char ch) {
	tx_state = SENDING_CHAR;
	usart_enable_tx_interrupt(USART1);
	usart_send(USART1, ch);
	//usart_send_blocking(USART1,ch);
}

static void uart_send_msg( char *msg){
	msg_addr = msg;
	tx_state = SENDING_MSG;
	usart_enable_tx_interrupt(USART1);
	usart_send(USART1, (*msg_addr));
	
}

static void gpio_setup(void) {

	/* Enable GPIOC clock. */
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Set GPIO8 (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);
	gpio_set(GPIOC,GPIO13); //start with led off
}


static void system_clock_setup(void) {
	
	rcc_clock_setup_in_hsi_out_24mhz();

}

static void TIM3_setup(void) {

	timer_reset(TIM3);
	rcc_periph_clock_enable(RCC_TIM3);
	timer_set_prescaler(TIM3, PRESCALER_TIM3 - 1); //this doesn't worg for frequency > 65Mhz
	timer_set_period(TIM3, PERIOD_TIM3 - 1); // period in ms
	
	// timer_enable_preload(TIM3);
	// timer_update_on_overflow(TIM3);
	// timer_enable_update_event(TIM3);
	//timer_disable_preload(TIM3);

	timer_enable_irq(TIM3,TIM_DIER_UIE); //update event interrupt
	nvic_clear_pending_irq(NVIC_TIM3_IRQ); //interrupt number for TIM3 (pag. 202)
	nvic_enable_irq(NVIC_TIM3_IRQ); //interrupt number for TIM3 (pag. 202)
	
	timer_enable_counter(TIM3);
	
}

void tim3_isr(void) {

	timer_clear_flag(TIM3, TIM_SR_UIF);

	//uart_send_msg("Starting\n\r");

	//gpio_toggle(GPIOC,GPIO13);	/* LED on */

}

void usart1_isr (void){

	if (usart_get_flag (USART1, USART_SR_RXNE )){ // Recieve flag
		char chr = usart_recv(USART1);
		usart_enable_tx_interrupt(USART1);
		usart_send(USART1, chr);
	}
	else if (usart_get_flag (USART1, USART_SR_TXE )){ // Transmit flag
		usart_disable_tx_interrupt(USART1);

		switch(tx_state){
			case SENDING_CHAR:
				tx_state = IDLE;
				break;
			case SENDING_MSG:
				if(*(++msg_addr) != 0){
					usart_enable_tx_interrupt(USART1);
					usart_send(USART1, (*msg_addr));
				}else{
					tx_state = IDLE;
				}
				break;
			default:
				tx_state = IDLE;
				break;
		}	
	}
	

}