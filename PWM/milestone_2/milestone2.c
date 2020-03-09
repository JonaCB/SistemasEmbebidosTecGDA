#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>

uint32_t core_clock_hz;

#define PRESCALER_TIM2 	64

static void gpio_setup(void);

static void timer2_setup(void);
static void timer3_setup(void);
static void system_clock_setup(void);
static void gpio_setup(void);

int main(void){
	system_clock_setup();
	gpio_setup();
	timer2_setup();
	timer3_setup();
	for(;;){
		
	}
	return 0;
}

static void timer2_setup(int period) {
	/* Enabling Timer 2 clock */
	rcc_periph_clock_enable(RCC_TIM2);
	/* To avoid starting up the counter and having a wrong count */
	timer_disable_counter(TIM2);
	timer_reset(TIM2);
	rcc_periph_reset_pulse(RST_TIM2);
	/* SET 	      TIMER2, NO MULTIPLICATION, EDGE ALIGNED,    UP COUNTING */
	timer_set_mode(TIM2,
			TIM_CR1_CKD_CK_INT,
			TIM_CR1_CMS_EDGE,
			TIM_CR1_DIR_UP);

	timer_enable_irq(TIM2,TIM_DIER_UIE);
	nvic_clear_pending_irq(NVIC_TIM2_IRQ);
	nvic_enable_irq(NVIC_TIM2_IRQ);

	timer_set_prescaler(TIM2, PRESCALER_TIM2-1);
	timer_enable_preload(TIM2);
	/* Timer continuous mode for it to keep running */
	timer_continuous_mode(TIM2);
	/* Set period to 1000 as previously defined */
	/* CHECK THIS PERIOD DEFINITION */
	timer_set_period(TIM2, period);

	timer_disable_oc_output(TIM2,TIM_OC2);
	timer_set_oc_mode(TIM2,TIM_OC2,TIM_OCM_PWM2);
	timer_enable_oc_output(TIM2, TIM_OC2);
	
	timer_set_oc_value(TIM2,TIM_OC2,0);

	timer_enable_counter(TIM2);
}

static void system_clock_setup(void){
	rcc_clock_setup_in_hsi_out_64mhz();
	core_clock_hz = 64000000;
}
