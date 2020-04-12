
#include "i2c_adc.h"
#include "../uc_i2c/uc_i2c.h"
#include <stdint.h>
#include <stdbool.h>

uint8_t ads1115_addr = 0b01001000;
uint8_t conversion_reg = 0x0;
uint8_t config_reg = 0x1;
uint8_t config_msb = 0b11000100;
uint8_t config_lsb = 0b10000011;

void adc_pin_setup(void){


}

void adc_setup(void){

    i2c_setup();

    i2c_start_addr(ads1115_addr,Write);
    i2c_write(config_reg);
    i2c_write(config_msb);
    i2c_write(config_lsb);
    i2c_stop();

}

uint16_t adc_read(void){

    uint8_t msb_data = 0x0;		// Read I2C byte
    uint8_t lsb_data = 0x0;		// Read I2C byte
    uint16_t data = 0;

    i2c_start_addr(ads1115_addr,Write);
    i2c_write_restart(conversion_reg,ads1115_addr);
    msb_data = i2c_read(false);
    lsb_data = i2c_read(true);
    data = (msb_data <<8) + lsb_data;
    int temp;
    temp = data * 210 / (65536/2);
    i2c_stop();

    return temp;

}