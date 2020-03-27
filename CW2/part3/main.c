
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include "miniprintf.h"
#include "string_conversion.h"


#define F_CLK 24000000
#define PRESCALER_TIM3 F_CLK / 1000 //TIM3 frequency = 24Mhz/24K = 1khz
#define PERIOD_TIM3 100 //500ms

#define MAX_TEMP_CMD 'x'
#define MIN_TEMP_CMD 'n'
#define VALUE_STR_LENGTH 11

static void gpio_setup(void);
static void system_clock_setup(void);
static void TIM3_setup(void);
static void usart_setup(void);
static void adc_setup(void);
static inline void uart_putc( char ch);
static int uart_printf(const char *format,...);

typedef enum {
  SEND_INFO,
  SEND_MAX_TEMP_MSG,
  SEND_MIN_TEMP_MSG,
  WAIT_TEMP
} globalSMType; 

typedef enum {
  WAIT_CONFIG_COMMAND,
  WAIT_VALUE
} rx_SMType; 

typedef enum {
  MAX_THR,
  MIN_THR
} thr_Type;

thr_Type threshold;

globalSMType global_state = SEND_INFO;
rx_SMType rx_state = WAIT_CONFIG_COMMAND;

char * error_msg = "\n\rError, invalid data was sent. Please send a valid temperature (integer numbers)\n\r\n\r";
char * big_int_msg = "\n\rThis number is bigger than the expected values\n\r\n\r";

typedef enum {
  ON,
  OFF,
} led_statusType; 

char * led_status[2] = {"On", "Off"};

led_statusType min_led = OFF;
led_statusType max_led = OFF;

int MaxTempTh = 30;
int MinTempTh = 29;

int count = 0;
int value_count = 0;

char temp_value [VALUE_STR_LENGTH];


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

	if (temp > MaxTempTh){
		gpio_clear(GPIOA,GPIO1); //on
		max_led = ON;
	}else{
		gpio_set(GPIOA,GPIO1); //off
		max_led = OFF;
	}
	if (temp < MinTempTh){
		gpio_clear(GPIOC,GPIO13); //on
		min_led = ON;
	}else{
		gpio_set(GPIOC,GPIO13); //off
		min_led = OFF;
	}


	switch(global_state){
		case SEND_INFO:
			if (count%5 == 0){
				uart_printf("Temp: %dC, MaxTempTh: %dC, MinTempTh: %dC, MaxTemp: %s, MinTemp: %s\n\r"
							, temp, MaxTempTh, MinTempTh, led_status[max_led],led_status[min_led]);
			}
			break;
		case SEND_MAX_TEMP_MSG:
			uart_printf("Set Max Temperature Threshold (C): ");
			global_state = WAIT_TEMP;
			break;
		case SEND_MIN_TEMP_MSG:
			uart_printf("Set Min Temperature Threshold (C): ");
			global_state = WAIT_TEMP;
			break;
		case WAIT_TEMP:
			break;
		default:
			global_state = SEND_INFO;
			break;
	}

	count++;
	

}

void usart1_isr (void){

	if (usart_get_flag (USART1, USART_SR_RXNE )){ // Recieve flag
		char chr = usart_recv(USART1);

		switch(rx_state){
			case WAIT_CONFIG_COMMAND:
				uart_printf("%c\n\r",chr);
				if (chr == MAX_TEMP_CMD){
					global_state = SEND_MAX_TEMP_MSG;
					rx_state = WAIT_VALUE;
					threshold = MAX_THR;
				}
				else if(chr == MIN_TEMP_CMD){
					global_state = SEND_MIN_TEMP_MSG;
					rx_state = WAIT_VALUE;
					threshold = MIN_THR;
				}
				else{
					uart_printf("\n\rCommand not valid. Type 'x' to configure max \n\rthreshold or 'n' to configure min threshold\n\r\n\r");
				}
				break;
			case WAIT_VALUE:
				if (chr == '.'){
					if (value_count > 0){
						temp_value[value_count] = 0;
						int val = atoi(temp_value);
						if (threshold == MAX_THR){
							MaxTempTh = val;
						} else{
							 MinTempTh = val;
						} 
						value_count = 0;
						uart_printf("%c\n\r",chr);
						global_state = SEND_INFO;
						rx_state = WAIT_CONFIG_COMMAND;
					}else{
						uart_printf("%c\n\r",chr);
						uart_printf(error_msg);
						global_state = (threshold == MAX_THR) ? SEND_MAX_TEMP_MSG: SEND_MIN_TEMP_MSG;
					}
				}else if ((chr >= '0') && (chr <= '9')){
					if (value_count > VALUE_STR_LENGTH - 2){
						value_count = 0;
						uart_printf("%c\n\r",chr);
						uart_printf(big_int_msg);
						global_state = (threshold == MAX_THR) ? SEND_MAX_TEMP_MSG: SEND_MIN_TEMP_MSG;
					}else{
						uart_printf("%c",chr);
						temp_value[value_count++] = chr;
					}
				}
				else{
					value_count = 0;
					uart_printf("%c\n\r",chr);
					uart_printf(error_msg);
					global_state = (threshold == MAX_THR) ? SEND_MAX_TEMP_MSG: SEND_MIN_TEMP_MSG;;
				}
				break;
			default:
				rx_state = WAIT_CONFIG_COMMAND;
				break;
		}

		
		
	}
	else if (usart_get_flag (USART1, USART_SR_TXE )){ // Transmit flag
		usart_disable_tx_interrupt(USART1);	
	}
	

}