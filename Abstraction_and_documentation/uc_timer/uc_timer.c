
#include "uc_timer.h"


void uc_timer_pwm_pin_setup(enum rcc_periph_clken gpio_clk, uint32_t gpio_port, uint16_t gpio_pin){
    	/******* Action Pin Setup ********/
	rcc_periph_clock_enable(gpio_clk);		// Need GPIOA clock

	gpio_set_mode(gpio_port,GPIO_MODE_OUTPUT_50_MHZ,	// High speed
		GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,gpio_pin);	// GPIOA1=TIM4.CH2
}


void uc_timer_setup(enum rcc_periph_clken timer_clk, uint32_t timer, uint32_t prescaler) {

	timer_reset(timer);
	rcc_periph_clock_enable(timer_clk);
	timer_set_prescaler(timer, prescaler - 1); //this doesn't worg for frequency > 65Mhz

}

void uc_timer_pwm_setup(enum rcc_periph_clken timer_clk, uint32_t timer, enum tim_oc_id channel, uint32_t prescaler) {

	/******* TIM4 as PWM setup ********/
	rcc_periph_clock_enable(timer_clk);		// Need TIM4 clock
	timer_disable_counter(timer);


	timer_set_mode(timer,
		TIM_CR1_CKD_CK_INT,
		TIM_CR1_CMS_EDGE,
		TIM_CR1_DIR_UP);
	timer_set_prescaler(timer,prescaler-1); 


	timer_disable_oc_output(timer,channel); 
	timer_set_oc_mode(timer,channel,TIM_OCM_PWM2); //PWM2 because we are using CH2
	timer_enable_oc_output(timer,channel); //Enabling CH2 as output

}

void uc_timer_config_period(uint32_t timer, uint32_t period){
    if(period == 0) period++;
    timer_set_period(timer, period-1); // period in ms
}

void uc_timer_pwm_config_duty_cycle(uint32_t timer, enum tim_oc_id channel, uint32_t duty_cycle){
    if(duty_cycle == 0) duty_cycle++;
    timer_set_oc_value(timer,channel, duty_cycle - 1); 
}


void uc_timer_enable_interrupt(uint32_t timer, uint8_t irqn){

	timer_enable_irq(timer,TIM_DIER_UIE); //update event interrupt
	nvic_clear_pending_irq(irqn); //interrupt number for TIM3 (pag. 202)
	nvic_enable_irq(irqn); //interrupt number for TIM3 (pag. 202)

}

void uc_timer_start(uint32_t timer){
    timer_enable_counter(timer);
}

void uc_timer_stop(uint32_t timer){
    timer_disable_counter(timer);
}
