#include "temp_sensor.h"

#include <libopencm3/stm32/adc.h>

void temp_sensor_setup(void){
	adc_pin_setup();
	adc_setup();
}

uint16_t temp_sensor_read(void){
	uint16_t adc_data = adc_read();
    return (uint16_t)(adc_convert_voltage(adc_data)*100.0);
}