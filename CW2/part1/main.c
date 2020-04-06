
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include "miniprintf.h"


#define F_CLK 24000000
#define PRESCALER_TIM3 F_CLK / 1000 //TIM3 frequency = 24Mhz/24K = 1khz
#define PERIOD_TIM3 100 //500ms

static void gpio_setup(void);
static void system_clock_setup(void);
static void TIM3_setup(void);
static void usart_setup(void);
static void adc_setup(void);
static inline void uart_putc( char ch);
static void uart_send_msg( char *msg);
static int uart_printf(const char *format,...);

typedef enum {
  IDLE,
  SENDING_MSG,
  SENDING_CHAR,
} TxSMType; 

TxSMType tx_state = IDLE;

char * msg_addr;

int MaxTempTh = 32;
int MinTempTh = 30;

int count = 0;


int main(void) {

	system_clock_setup();
	gpio_setup();
	usart_setup();
	adc_setup();
	TIM3_setup();

	for (;;) {

	}

	return 0;
}

static void adc_setup(void){
	rcc_periph_clock_enable(RCC_GPIOA);		// Enable GPIOA for ADC
	gpio_set_mode(GPIOA,
		GPIO_MODE_INPUT,
		GPIO_CNF_INPUT_ANALOG,
		GPIO0);


	rcc_periph_clock_enable(RCC_ADC1);
	adc_power_off(ADC1);
	rcc_periph_reset_pulse(RST_ADC1);
	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV2);	// Set. 12MHz, Max. 14MHz
	adc_set_dual_mode(ADC_CR1_DUALMOD_IND);
	adc_disable_scan_mode(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_set_sample_time(ADC1, ADC_CHANNEL0, ADC_SMPR_SMP_239DOT5CYC);

	adc_power_on(ADC1);
	adc_reset_calibration(ADC1);
	adc_calibrate_async(ADC1);
	while ( adc_is_calibrating(ADC1) );

}


static uint16_t read_adc(uint8_t channel) {

	adc_set_sample_time(ADC1,channel,ADC_SMPR_SMP_239DOT5CYC);
	adc_set_regular_sequence(ADC1,1,&channel);
	adc_start_conversion_direct(ADC1);
	while ( !adc_eoc(ADC1) );
	return adc_read_regular(ADC1);
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

	usart_set_baudrate(USART1,38400);
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
	usart_send_blocking(USART1,ch);
}

static int uart_printf(const char *format,...) {
	va_list args;
	int rc;

	va_start(args,format);
	rc = mini_vprintf_cooked(uart_putc,format,args);
	va_end(args);
	return rc;
}

static void gpio_setup(void) {

	/* Enable GPIOC clock. */
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Set GPIO8 (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);
	gpio_set(GPIOC,GPIO13); //start with led off


	rcc_periph_clock_enable(RCC_GPIOA);		// Need GPIOA clock
	gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,GPIO1);
	gpio_set(GPIOA,GPIO1); //start with led off


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

	int mV, temp;
	mV = read_adc(ADC_CHANNEL0) * 330 / 4095;
	temp = mV;

	if (temp >= MaxTempTh){
		gpio_clear(GPIOA,GPIO1); //on
	}else{
		gpio_set(GPIOA,GPIO1); //off
	}
	if (temp <= MinTempTh){
		gpio_clear(GPIOC,GPIO13); //on
	}else{
		gpio_set(GPIOC,GPIO13); //off
	}

	if (count%5 == 0){
		uart_printf("Temp: %dC, MaxTempTh: %dC, MinTempTh: %dC,\n\r", temp, MaxTempTh, MinTempTh);
	}

	
	count++;
	

}

void usart1_isr (void){

	if (usart_get_flag (USART1, USART_SR_RXNE )){ // Recieve flag
		char chr = usart_recv(USART1);
		uart_printf("%c\n\r",chr);
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