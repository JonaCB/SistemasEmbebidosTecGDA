#include "temp_sensor.h"

void temp_sensor_setup(void){
    adc_pin_setup();
    adc_setup();

}

uint16_t temp_sensor_read(void){
    uint16_t data = adc_read();
    float voltage = adc_convert_voltage(data);
    return (uint16_t)(voltage*100.0); //convert mv to temperature (LM35 sensor is linear, 1mv = 1C)

}
