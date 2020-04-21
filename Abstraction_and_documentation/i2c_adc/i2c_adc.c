/// @file i2c_adc.c
//  Copyright 2020 Copyright Equipo 2
#include "i2c_adc.h"
#include "../uc_i2c/uc_i2c.h"
#include <stdint.h>
#include <stdbool.h>

uint8_t ads1115_addr = 0b01001000;                /// <I2C ADC Address.
uint8_t conversion_reg = 0x0;                     /// <Conversion register
// address.
uint8_t config_reg = 0x1;                         /// <Configuration register
// address.
uint8_t config_msb = 0b11000010;  // 0b11000100;     /// <Start a single
// conversion (when in power-down state). AINP = AIN0 and AINN = GND,
// FSR = ±4.096 V, Continuous-conversion mode.
uint8_t config_lsb = 0b10000011;                  /// <128 SPS, Disable
// comparator and set ALERT/RDY pin to high-impedance.

/**
 * Sets up the ADC pins needed.
 */
void adc_pin_setup(void) {}

/**
 * Sets up the adc using I2C communication.
 */
void adc_setup(void) {
    i2c_setup();
    i2c_start_addr(ads1115_addr, Write);
    i2c_write(config_reg);
    i2c_write(config_msb);
    i2c_write(config_lsb);
    i2c_stop();
}

/**
 * Reads the value of the ADC.
 * @param[out] data
 */
uint16_t adc_read(void) {
    uint8_t msb_data = 0x0;        /// MSB Read I2C byte
    uint8_t lsb_data = 0x0;        /// LSB Read I2C byte
    uint16_t data = 0;        /// Full data

    i2c_start_addr(ads1115_addr, Write);
    i2c_write_restart(conversion_reg, ads1115_addr);
    msb_data = i2c_read(false);
    lsb_data = i2c_read(true);
    i2c_stop();
    data = (msb_data <<8) + lsb_data;
    // int temp;
    // temp = data * 210 / (65536/2);
    return data;
}

/**
 * Scales the adc value to a given voltage scale.
 * @param[in] data
 * @param[out] scaled_value
 */
float adc_convert_voltage(uint16_t data) {
    float scale = 125E-6;  // 62.5E-6;
    return ((float)(data))*scale;
}
