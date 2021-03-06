//  Copyright 2020 Copyright Equipo 2
#ifndef TEMP_SENSOR_H_   /* Include guard */
#define TEMP_SENSOR_H_

#include <stdint.h>

#define I2C_ADC

#ifdef I2C_ADC
#include "../i2c_adc/i2c_adc.h"
#else
#include "../uc_adc/uc_adc.h"
#endif

void temp_sensor_setup(void);
uint16_t temp_sensor_read(void);

#endif  // ABSTRACTION_AND_DOCUMENTATION_TEMP_SENSOR_TEMP_SENSOR_H_
