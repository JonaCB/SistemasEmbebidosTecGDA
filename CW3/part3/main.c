
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>
#include "miniprintf.h"
#include "string_conversion.h"


#define F_CLK 24000000 // 24Mhz
#define PRESCALER_TIM3 F_CLK / 1000 //TIM3 frequency = 24Mhz/24K = 1khz
#define PERIOD_TIM3 100 //500ms
#define PRESCALER_TIM2 1 //TIM2 frequency = 64Mhz /2 = 32Mhz
#define PWM_FREQUENCY 10000 //PWM Frequency = 10kHz
#define PWM_PERIOD F_CLK/PRESCALER_TIM2/PWM_FREQUENCY //2400 counts,  couter 100 - 65000


#define CMD_MAX_TEMP 'x'
#define CMD_MIN_TEMP 'n'

#define CMD_DOWN_THR_MIN 's' 
#define CMD_UP_THR_MIN 'w'
#define CMD_DOWN_THR_MAX 'a'
#define CMD_UP_THR_MAX 'd'
#define CMD_UP_DIM 'c'
#define CMD_DOWN_DIM 'z'
#define CMD_MENU 'm'

#define THR_STEP 1
#define DIM_STEP 10


#define MAX_DIM_LIMIT 100
#define MIN_DIM_LIMIT 0

#define MAX_THR_LIMIT 50
#define MIN_THR_LIMIT 0


#define VALUE_STR_LENGTH 11

static void gpio_setup(void);
static void system_clock_setup(void);
static void TIM2_setup(void);
static void TIM3_setup(void);
static void usart_setup(void);
static void adc_setup(void);
static inline void uart_putc( char ch);
static int uart_printf(const char *format,...);
uint16_t get_pwm_percentage_counts(uint16_t percentage);

typedef enum {
  SEND_INFO,
  WAIT_INPUT
} globalSMType; 

typedef enum {
  WAIT_CONFIG_COMMAND,
  WAIT_VALUE,
  WAIT_MENU_NUMBER
} rx_SMType; 

typedef enum {
  MAX_THR,
  MIN_THR,
  DIM_VALUE
} thr_Type;

thr_Type threshold;

globalSMType global_state = SEND_INFO;
rx_SMType rx_state = WAIT_CONFIG_COMMAND;

char * error_msg = "\n\rError, invalid data was sent. Please send a valid temperature (integer numbers)\n\r\n\r";
char * big_int_msg = "\n\rThis number is bigger than the expected values\n\r\n\r";
char * thr_error_msg = "\n\rPlease enter a number between %d and %d\n\r\n\r";
char * dim_error_msg = "\n\rPlease enter a number between %d and %d\n\r\n\r";
char * max_thr_msg = "Set Max Temperature Threshold (C): ";
char * min_thr_msg = "Set Min Temperature Threshold (C): ";
char * dim_msg = "Set Dimming Percentage (%%): ";

typedef enum {
  ON,
  OFF,
} led_statusType; 

char * led_status[2] = {"On", "Off"};

led_statusType min_led = OFF;
led_statusType max_led = OFF;

int MaxTempTh = 35;
int MinTempTh = 25;

int dimPercentage = 50;

int count = 0;
int value_count = 0;

char temp_value [VALUE_STR_LENGTH];


int main(void) {

	system_clock_setup();
	gpio_setup();
	TIM2_setup();
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


	// rcc_periph_clock_enable(RCC_GPIOA);		// Need GPIOA clock
	// gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_2_MHZ,
	// 	      GPIO_CNF_OUTPUT_PUSHPULL,GPIO1);
	// gpio_set(GPIOA,GPIO1); //start with led off


	rcc_periph_clock_enable(RCC_GPIOB);		// Need GPIOA clock
	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,GPIO8);

	gpio_set_mode(GPIOB,GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,GPIO5);
	gpio_set(GPIOB,GPIO5); //start with led off
	gpio_set(GPIOB,GPIO8); //start with led off


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


static void TIM2_setup(void) {

	// PA1 == TIM2.CH2	
	rcc_periph_clock_enable(RCC_GPIOA);		// Need GPIOA clock
	gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_50_MHZ,	// High speed
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO1);	// GPIOA1=TIM2.CH2

	rcc_periph_clock_enable(RCC_TIM2);		// Need TIM2 clock

	// TIM2:
	timer_disable_counter(TIM2);
	rcc_periph_reset_pulse(RST_TIM2);

	timer_set_mode(TIM2,
		TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE,
		TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM2,PRESCALER_TIM2-1); 
	timer_enable_preload(TIM2);
	timer_continuous_mode(TIM2);
	timer_set_period(TIM2,PWM_PERIOD);

	timer_disable_oc_output(TIM2,TIM_OC2); 
	timer_set_oc_mode(TIM2,TIM_OC2,TIM_OCM_PWM2); //PWM2 because we are using CH2
	timer_enable_oc_output(TIM2,TIM_OC2); //Enabling CH2 as output

	timer_set_oc_value(TIM2,TIM_OC2,get_pwm_percentage_counts(dimPercentage) - 1); //Setting initial value tu 50%
	timer_enable_counter(TIM2);

}

uint16_t get_pwm_percentage_counts(uint16_t percentage){
	uint16_t counts = (percentage*PWM_PERIOD) / 100;
	if (counts == 0){
		counts++; //this prevents the counter to have a negative value when we substract 1
	}
	return counts;
}

void tim3_isr(void) {

	timer_clear_flag(TIM3, TIM_SR_UIF);

	int mV, temp;
	mV = read_adc(ADC_CHANNEL0) * 330 / 4095;
	temp = mV;

	if (temp > MaxTempTh){
		gpio_clear(GPIOB,GPIO8); //on
		max_led = ON;
	}else{
		gpio_set(GPIOB,GPIO8); //off
		max_led = OFF;
	}
	if (temp < MinTempTh){
		gpio_clear(GPIOB,GPIO5); //on
		min_led = ON;
	}else{
		gpio_set(GPIOB,GPIO5); //off
		min_led = OFF;
	}


	switch(global_state){
		case SEND_INFO:
			if (count%5 == 0){
				uart_printf("Temp: %dC, MaxTempTh: %dC, MinTempTh: %dC, MaxTemp: %s, MinTemp: %s, Led Intensity: %d%%\n\r"
							, temp, MaxTempTh, MinTempTh, led_status[max_led],led_status[min_led], dimPercentage);
			}
			break;
		case WAIT_INPUT:
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
				switch(chr){
					case CMD_MAX_TEMP:
						uart_printf(max_thr_msg);
						global_state = WAIT_INPUT;
						rx_state = WAIT_VALUE;
						threshold = MAX_THR;
						break;
					case CMD_MIN_TEMP:
						uart_printf(min_thr_msg);
						global_state = WAIT_INPUT;
						rx_state = WAIT_VALUE;
						threshold = MIN_THR;
						break;
					case CMD_UP_DIM:
						dimPercentage += DIM_STEP;
						if(dimPercentage > MAX_DIM_LIMIT) dimPercentage = MAX_DIM_LIMIT;
						timer_set_oc_value(TIM2,TIM_OC2,get_pwm_percentage_counts(dimPercentage) - 1);
						break;
					case CMD_DOWN_DIM:
						dimPercentage -= DIM_STEP;
						if (dimPercentage < MIN_DIM_LIMIT) dimPercentage = MIN_DIM_LIMIT;
						timer_set_oc_value(TIM2,TIM_OC2,get_pwm_percentage_counts(dimPercentage) - 1);
						break;
					case CMD_UP_THR_MAX:
						if(MaxTempTh + 1 <= MAX_THR_LIMIT) MaxTempTh += THR_STEP;
						break;
					case CMD_DOWN_THR_MAX:
						if(MaxTempTh > MinTempTh + 1) MaxTempTh -= THR_STEP;
						break;
					case CMD_UP_THR_MIN:
						if(MinTempTh + 1 < MaxTempTh) MinTempTh += THR_STEP;
						break;
					case CMD_DOWN_THR_MIN:
						if (MinTempTh -1  >= MIN_THR_LIMIT) MinTempTh -= THR_STEP;
						break;
					case CMD_MENU:
						uart_printf("\n\rEnter the number of the parameter you want to configure:\n\r%s%s%s\n\rParameter: ",\
									"1 - Max Temperature Threshold\n\r",\
									"2 - Min Temperature Threshold\n\r",\
									"3 - Dimming Percentage\n\r");
						global_state = WAIT_INPUT;
						rx_state = WAIT_MENU_NUMBER;
						break;
					default:
						uart_printf("\n\rCommand not valid. Type 'x' to configure max \n\rthreshold or 'n' to configure min threshold\n\r\n\r");
						break;
				}

				break;
			case WAIT_VALUE:
				if (chr == '\r'){
					if (value_count > 0){
						temp_value[value_count] = 0;
						int val = atoi(temp_value);
						value_count = 0;
						uart_printf("%c\n\r",chr);
						switch(threshold){
							case( MAX_THR):
								if((val > MAX_THR_LIMIT) || (val <= MinTempTh)){
									//return error
									uart_printf("\n\r");
									uart_printf(thr_error_msg, MinTempTh + 1, MAX_THR_LIMIT);
									uart_printf(max_thr_msg);
								}
								else{
									MaxTempTh = val;
									global_state = SEND_INFO;
									rx_state = WAIT_CONFIG_COMMAND;
									}
								break;
							case(MIN_THR):
								if((val < MIN_THR_LIMIT) || (val >= MaxTempTh)){
									//return error
									uart_printf("\n\r");
									uart_printf(thr_error_msg, MIN_THR_LIMIT, MaxTempTh - 1);
									uart_printf(min_thr_msg);
								}else{
									MinTempTh = val;
									global_state = SEND_INFO;
									rx_state = WAIT_CONFIG_COMMAND;
								}
								break;
							case(DIM_VALUE):
								if((val < MIN_DIM_LIMIT) || (val > MAX_DIM_LIMIT)){
									//return error
									uart_printf("\n\r");
									uart_printf(dim_error_msg, MIN_DIM_LIMIT, MAX_DIM_LIMIT);
									uart_printf(dim_msg);
								}else{
									dimPercentage = val;
									timer_set_oc_value(TIM2,TIM_OC2,get_pwm_percentage_counts(dimPercentage) - 1);
									global_state = SEND_INFO;
									rx_state = WAIT_CONFIG_COMMAND;
								}
								break;
							default:
								break;
						}

						
					}else{
						uart_printf("%c\n\r",chr);
						uart_printf(error_msg);
						switch(threshold){
							case MAX_THR:
								uart_printf(max_thr_msg);
								break;
							case MIN_THR:
							 	uart_printf(min_thr_msg);	
								break;
							case DIM_VALUE:
								uart_printf(dim_msg);	
								break;
							default:
								break;
						}
					}
				}else if ((chr >= '0') && (chr <= '9')){
					if (value_count > VALUE_STR_LENGTH - 2){
						value_count = 0;
						uart_printf("%c\n\r",chr);
						uart_printf(big_int_msg);
						switch(threshold){
							case MAX_THR:
								uart_printf(max_thr_msg);
								break;
							case MIN_THR:
							 	uart_printf(min_thr_msg);	
								break;
							case DIM_VALUE:
								uart_printf(dim_msg);	
								break;
							default:
								break;
						}
					
					}else{ //is a number
						uart_printf("%c",chr);
						temp_value[value_count++] = chr;
					}
				}
				else{
					value_count = 0;
					uart_printf("%c\n\r",chr);
					uart_printf(error_msg);
					switch(threshold){
							case MAX_THR:
								uart_printf(max_thr_msg);
								break;
							case MIN_THR:
							 	uart_printf(min_thr_msg);	
								break;
							case DIM_VALUE:
								uart_printf(dim_msg);	
								break;
							default:
								break;
						}
				}
				break;
			case WAIT_MENU_NUMBER:
				uart_printf("%c\r\n", chr);
				switch(chr){
					case '1':
						uart_printf(max_thr_msg);
						global_state = WAIT_INPUT;
						rx_state = WAIT_VALUE;
						threshold = MAX_THR;
						break;
					case '2':
						uart_printf(min_thr_msg);
						global_state = WAIT_INPUT;
						rx_state = WAIT_VALUE;
						threshold = MIN_THR;
						break;
					case '3':
						uart_printf(dim_msg);
						global_state = WAIT_INPUT;
						rx_state = WAIT_VALUE;
						threshold = DIM_VALUE;
						break;
					default:
						uart_printf("Please select a number in the list.\r\n%s%s%s\n\rParameter: ",\
									"1 - Max Temperature Threshold\n\r",\
									"2 - Min Temperature Threshold\n\r",\
									"3 - Dimming Percentage\n\r");
						break;
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