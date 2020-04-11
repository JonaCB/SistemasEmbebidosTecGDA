#include "temp_sensor.h"

#include <libopencm3/stm32/adc.h>

void temp_sensor_setup(void){
	adc_pin_setup();
	adc_setup();
}

uint16_t temp_sensor_read(void){
    return adc_read(ADC_CHANNEL0);
}