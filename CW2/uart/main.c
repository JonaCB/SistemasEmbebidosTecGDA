
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>

#define F_CLK 24000000
#define PRESCALER_TIM3 F_CLK / 1000 //TIM3 frequency = 24Mhz/24K = 1khz
#define PERIOD_TIM3 100 //100ms

static void gpio_setup(void);
static void system_clock_setup(void);
static void TIM3_setup(void);
static void usart_setup(void);
static inline void uart_putc( char ch);
static void uart_send_word( char *word);


uint8_t c = '0' - 1;

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

	usart_set_baudrate(USART1,38400);
	usart_set_databits(USART1,8);
	usart_set_stopbits(USART1,USART_STOPBITS_1);
	usart_set_mode(USART1,USART_MODE_TX);
	usart_set_parity(USART1,USART_PARITY_NONE);
	usart_set_flow_control(USART1,USART_FLOWCONTROL_NONE);
	usart_enable(USART1);
}

static inline void uart_putc( char ch) {
	usart_send_blocking(USART1,ch);
}

static void uart_send_word( char *word){
	uint8_t i = 0;
	do{
		uart_putc(word[i++]); 
		//i++;
	}while(word[i] != 0);
	
}

static void gpio_setup(void) {

	/* Enable GPIOC clock. */
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Set GPIO8 (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);
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

	gpio_toggle(GPIOC,GPIO13);	/* LED on */

	// if ( ++c >= 'Z' ) {
	// 	uart_putc(c);
	// 	uart_putc('\r');
	// 	uart_putc('\n');
	// 	c = '0' - 1;
	// } else	{
	// 	uart_putc(c);
	// }

	uart_send_word("Hola\n\r");


}