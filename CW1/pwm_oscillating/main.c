#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>

#define F_CLK 64000000
#define PRESCALER_TIM2 2 //TIM2 frequency = 64Mhz /2 = 32Mhz
#define PWM_FREQUENCY 100000 //PWM Frequency = 100kHz
#define PWM_PERIOD F_CLK/PRESCALER_TIM2/PWM_FREQUENCY //320 counts
#define PRESCALER_TIM3 64 //TIM3 frequency = 64Mhz/64 = 1Mhz
#define F_TIM3 F_CLK/PRESCALER_TIM3 

#define F0 1 //initial frequency of 1Hz
//we multiply this frequency by 255

#define F_MIN F0
#define F_MAX 10*F0

static void TIM2_setup(void);
static void TIM3_setup(void);
static void gpio_setup(void);
static void system_clock_setup(void);
uint16_t get_pwm_percentage_counts(uint16_t percentage);
uint16_t get_divided_frequency_counts(uint16_t frequency);

typedef enum  {
  UP,
  DOWN
} directionType; 

float f = F_MIN; //initial frequency of 1Hz
directionType direction = UP; //initial frequency of 1Hz


//sine function generated in python
/*
import numpy as np
f = 100
time= np.arange(0, 1/f, (1/f)/255);
amplitude   = np.sin(2*np.pi*f*time)
x = (amplitude +1) / 2 * 100
print(x.astype(int))
*/
static const uint8_t sine_function[255] = { 50, 51, 52, 53, 54, 56, 57, 58, 59, 60, 62, 63, 64, 65, 66, 68, 69,
										70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86,
										87, 87, 88, 89, 90, 90, 91, 92, 92, 93, 94, 94, 95, 95, 96, 96, 97,
										97, 97, 98, 98, 98, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99,
										99, 99, 99, 99, 98, 98, 98, 98, 97, 97, 96, 96, 96, 95, 95, 94, 93,
										93, 92, 92, 91, 90, 89, 89, 88, 87, 86, 85, 85, 84, 83, 82, 81, 80,
										79, 78, 77, 76, 75, 74, 73, 72, 70, 69, 68, 67, 66, 65, 63, 62, 61,
										60, 59, 57, 56, 55, 54, 53, 51, 50, 49, 48, 46, 45, 44, 43, 42, 40,
										39, 38, 37, 36, 34, 33, 32, 31, 30, 29, 27, 26, 25, 24, 23, 22, 21,
										20, 19, 18, 17, 16, 15, 14, 14, 13, 12, 11, 10, 10,  9,  8,  7,  7,
										6,  6,  5,  4,  4,  3,  3,  3,  2,  2,  1,  1,  1,  1,  0,  0,  0,
										0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  2,
										2,  2,  3,  3,  4,  4,  5,  5,  6,  7,  7,  8,  9,  9, 10, 11, 12,
										12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28,
										29, 30, 31, 33, 34, 35, 36, 37, 39, 40, 41, 42, 43, 45, 46, 47, 48};

uint32_t counter = 0;

int main(void) {

	system_clock_setup();
	gpio_setup();
	TIM3_setup();
	TIM2_setup();
	for (;;) {
	}

	return 0;
}

static void gpio_setup(void) {

	// /* Enable GPIOC clock. */
	// rcc_periph_clock_enable(RCC_GPIOC);

	// /* Set GPIO8 (in GPIO port C) to 'output push-pull'. */
	// gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,
	// 	      GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);

	rcc_periph_clock_enable(RCC_AFIO);		// Need AFIO clock

	// PA1 == TIM2.CH2	
	rcc_periph_clock_enable(RCC_GPIOA);		// Need GPIOA clock
	gpio_primary_remap(
		AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_OFF,	// Optional
		AFIO_MAPR_TIM2_REMAP_NO_REMAP);		// This is default: TIM2.CH2=GPIOA1
	gpio_set_mode(GPIOA,GPIO_MODE_OUTPUT_50_MHZ,	// High speed
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,GPIO1);	// GPIOA1=TIM2.CH2
}


static void system_clock_setup(void) {
	
	rcc_clock_setup_in_hsi_out_64mhz();

}


static void TIM2_setup(void) {

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

	timer_set_oc_value(TIM2,TIM_OC2,get_pwm_percentage_counts(sine_function[counter = 0]) - 1); //Setting initial value tu 50%
	timer_enable_counter(TIM2);

}

uint16_t get_pwm_percentage_counts(uint16_t percentage){
	uint16_t counts = (percentage*PWM_PERIOD) / 100;
	if (counts == 0){
		counts++; //this prevents the counter to have a negative value when we substract 1
	}
	return counts;
}

static void TIM3_setup(void) {

	timer_reset(TIM3);
	rcc_periph_clock_enable(RCC_TIM3);
	timer_set_prescaler(TIM3, PRESCALER_TIM3 - 1); //this doesn't worg for frequency > 65Mhz
	timer_set_period(TIM3, get_divided_frequency_counts((uint16_t)f) - 1); // period in ms
	
	// timer_enable_preload(TIM3);
	// timer_update_on_overflow(TIM3);
	// timer_enable_update_event(TIM3);
	//timer_disable_preload(TIM3);

	timer_enable_irq(TIM3,TIM_DIER_UIE); //update event interrupt
	nvic_clear_pending_irq(NVIC_TIM3_IRQ); //interrupt number for TIM3 (pag. 202)
	nvic_enable_irq(NVIC_TIM3_IRQ); //interrupt number for TIM3 (pag. 202)
	
	timer_enable_counter(TIM3);
	
}

uint16_t get_divided_frequency_counts(uint16_t frequency){
	return F_TIM3/(frequency*255);
}

void tim3_isr (void){

	counter = (counter+1) % 255;
	if(counter == 0){
		switch(direction){
			case UP:
				f += 0.1f*f;
				if (f>F_MAX){
					f = F_MAX;
					direction = DOWN;
				}
				break;
			case DOWN:
				f -= 0.1f*f;
				if (f<F_MIN){
					f = F_MIN;
					direction = UP;
				}
				break;
		}

		timer_set_period(TIM3, get_divided_frequency_counts((uint16_t)f) - 1); // period in ms
		//timer_generate_event(TIM3, TIM_EGR_UG );
		//gpio_toggle(GPIOC,GPIO13);	/* LED toogle */
	}
	timer_clear_flag(TIM3, TIM_SR_UIF);
	timer_set_oc_value(TIM2,TIM_OC2,get_pwm_percentage_counts(sine_function[counter]) - 1);
}


