#ifndef TEMP_SENSOR_H_   /* Include guard */
#define TEMP_SENSOR_H_


#include <stdint.h>
#include "../uc_adc/uc_adc.h"

void temp_sensor_setup(void);
uint16_t temp_sensor_read(void);




#endif // TEMP_SENSOR_H_