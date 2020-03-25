#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/adc.h>

#define PRESCALER_TIM2 6400 //TIM2 FREQ = 64MHz/64 = 10kHz

static void gpio_setup(void);
static void system_clock_setup(void);
static void TIM2_setup(void);
static void ADC_setup(void);
static void system_clock_setup(void);

int main(void)
{
	TIM2_setup();
	ADC_setup();
	for(;;);
	return 0;
}

static void ADC_setup(void){
	// CHECK WHICH CLOCKS TO CONFIGURE
	rcc_peripheral_enable_clock(&RCC_APB2ENR,RCC_APB2ENR_ADC1EN);
	adc_power_off(ADC1);
	
	rcc_peripheral_reset(&RCC_APB2RSTR,RCC_APB2RSTR_ADC1RST);
	rcc_peripheral_clear_reset(&RCC_APB2RSTR,RCC_APB2RSTR_ADC1RST);
	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV6);	// Set. 12MHz, Max. 14MHz
	adc_set_dual_mode(ADC_CR1_DUALMOD_IND);		// Independent mode
	// DISABLES SCANNING OF THE 4 ADC CHANNELS
	adc_disable_scan_mode(ADC1);
	// 
	adc_set_right_aligned(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_set_sample_time(ADC1,ADC_CHANNEL_TEMP,ADC_SMPR_SMP_239DOT5CYC);
	adc_set_sample_time(ADC1,ADC_CHANNEL_VREF,ADC_SMPR_SMP_239DOT5CYC);
	// ENABLES ADC END OF CONVERSION INTERRUPT
	adc_enable_eoc_interrupt(ADC1);
	adc_power_on(ADC1);
	adc_reset_calibration(ADC1);
	adc_calibrate_async(ADC1);
}

static void GPIO_setup(void){
	// ENABLE CLOCK
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_set_mode(GPIOA,
			GPIO_MODE_INPUT,
			GPIO_CNF_INPUT_ANALOG,
			GPIO0|GPIO1);
}

static void system_clock_setup(void){
	rcc_clock_setup_in_hsi_out_64mhz();
}

static void TIM2_setup(void){
	// RESET TIMER
	//timer_reset(TIM2);
	// ENABLE CLOCK FOR TIMER 2
	rcc_periph_clock_enable(RCC_TIM2);
	// SET CLOCK PRESCALER FOR TIMER 2 TO 6399 (0-6399)
	timer_set_prescaler(TIM2,PRESCALER_TIM2-1);
	// SET TIMER COUNT UP TO 1000 us
	timer_set_period(TIM2, 1000);
	// ENABLE IRQ FOR TIM2 TO TURN ON ADC
	timer_enable_irq(TIM2,TIM_DIER_UIE);
	// INTERRUPT NUMBER FOR TIM2 (P. 202)
	nvic_clear_pending_irq(NVIC_TIM2_IRQ);
	nvic_enable_irq(NVIC_TIM2_IRQ);
	
}

void tim2_isr(void){
	timer_clear_flag(TIM2, TIM_SR_UIF);
	// TURN ON ADC CONVERSIONS
	adc_start_conversion_regular(ADC1);
}

void adc1_isr(void){
	
}
