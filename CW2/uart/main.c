
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>

uint32_t core_clock_hz;

static void timer_setup(void);
static void gpio_setup(void);
static void system_clock_setup(void);
static void delay_ms(uint32_t ms);
static void usart_setup(void);
static inline void uart_putc(char ch);

int main(void) {

	system_clock_setup();
	gpio_setup();
	usart_setup();
	timer_setup();

	uint8_t c = '0' - 1;

	for (;;) {
		gpio_toggle(GPIOC,GPIO13);	/* LED on */
		delay_ms(100);

		if ( ++c >= 'Z' ) {
			uart_putc(c);
			uart_putc('\r');
			uart_putc('\n');
			c = '0' - 1;
		} else	{
			uart_putc(c);
		}

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

static inline void uart_putc(char ch) {
	usart_send_blocking(USART1,ch);
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
  	core_clock_hz = 24000000;

}

static void timer_setup(void) {

	timer_reset(TIM2);
	rcc_periph_clock_enable(RCC_TIM2);
	timer_set_prescaler(TIM2, core_clock_hz / 1000);
}

static void delay_ms(uint32_t ms){

	timer_set_period(TIM2, ms);
	timer_enable_counter(TIM2);
	while (!timer_get_flag(TIM2, TIM_SR_UIF)) {}; //wait for the counter
	timer_clear_flag(TIM2, TIM_SR_UIF);
	timer_disable_counter(TIM2);
}
