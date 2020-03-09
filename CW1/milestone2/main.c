#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>

#define F_CLK 64000000
#define PRESCALER_TIM3 6400 //TIM3 frequency = 64Mhz/64 = 10khz
#define F_TIM3 F_CLK/PRESCALER_TIM3 

#define F0 1 //initial frequency of 1Hz
//we multiply this frequency by 255

#define F_MIN F0
#define F_MAX 10*F0

static void TIM3_setup(void);
static void gpio_setup(void);
static void system_clock_setup(void);
uint16_t get_divided_frequency_counts(uint16_t frequency);

typedef enum  {
  UP,
  DOWN
} directionType; 

float f = F_MIN; //initial frequency of 1Hz
directionType direction = UP; //initial frequency of 1Hz


int main(void) {

	system_clock_setup();
	gpio_setup();
	TIM3_setup();
	for (;;) {
	}

	return 0;
}

static void gpio_setup(void) {

	/* Enable GPIOC clock. */
	rcc_periph_clock_enable(RCC_GPIOC);

	/* Set GPIO8 (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOC,GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL,GPIO13);

}


static void system_clock_setup(void) {
	
	rcc_clock_setup_in_hsi_out_64mhz();

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
	return F_TIM3/(frequency);
}

void tim3_isr (void){

	timer_clear_flag(TIM3, TIM_SR_UIF);

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
	gpio_toggle(GPIOC,GPIO13);	/* LED toogle */


}


