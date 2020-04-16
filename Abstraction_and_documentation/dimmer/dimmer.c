#include "dimmer.h"
#include "../uc_timer/uc_timer.h"
#include "../system_common/system_common.h"

uint16_t get_pwm_percentage_counts(uint16_t percentage);

/**
 * Initializes the timer, sets it up as a PWM and starts the timer.
 */
void dimmer_setup(void) {
	uc_timer_pwm_pin_setup(PWM_PIN_PORT_CLOCK, PWM_PIN_PORT, PWM_PIN);
	uc_timer_pwm_setup(PWM_TIMER_CLOCK, PWM_TIMER, PWM_TIMER_CHANNEL, PWM_PRESCALER);
	uc_timer_config_period(PWM_TIMER, PWM_PERIOD);
	//uc_timer_pwm_config_duty_cycle(PWM_TIMER, PWM_TIMER_CHANNEL, get_pwm_percentage_counts(dimPercentage));
	uc_timer_start(PWM_TIMER);

}

/**
 * Updates the dimming intensity of the light.
 * @param[in] percentage
 */
void dimmer_update_percentage(uint16_t percentage){
    uc_timer_pwm_config_duty_cycle(PWM_TIMER, PWM_TIMER_CHANNEL, get_pwm_percentage_counts(percentage));
}

/**
 * Calculates a percentage of dimming given a constant.
 * @param[in] percentage
 * @param[out] counts
 */
uint16_t get_pwm_percentage_counts(uint16_t percentage){
	uint16_t counts = (percentage*PWM_PERIOD) / 100;
	if (counts == 0){
		counts++; //this prevents the counter to have a negative value when we substract 1
	}
	return counts;
}
