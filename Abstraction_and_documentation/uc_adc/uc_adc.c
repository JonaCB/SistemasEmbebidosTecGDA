/// @file uc_adc.c
//  Copyright 2020 Copyright Equipo 2
#include "uc_adc.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>

/**
 * Sets up the ADC peripheral pin ports needed.
 */
void adc_pin_setup(void) {
    rcc_periph_clock_enable(RCC_GPIOA);        // Enable GPIOA for ADC
    gpio_set_mode(GPIOA,
        GPIO_MODE_INPUT,
        GPIO_CNF_INPUT_ANALOG,
        GPIO0);
}

/**
 * Sets up the ADC peripheral.
 */
void adc_setup(void) {
    rcc_periph_clock_enable(RCC_ADC1);
    adc_power_off(ADC1);
    rcc_periph_reset_pulse(RST_ADC1);
    rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV2);    // Set. 12MHz, Max. 14MHz
    adc_set_dual_mode(ADC_CR1_DUALMOD_IND);
    adc_disable_scan_mode(ADC1);
    adc_set_single_conversion_mode(ADC1);
    adc_set_sample_time(ADC1, ADC_CHANNEL0, ADC_SMPR_SMP_239DOT5CYC);

    adc_power_on(ADC1);
    adc_reset_calibration(ADC1);
    adc_calibrate_async(ADC1);
    while ( adc_is_calibrating(ADC1) ) {}
}

/**
 * Reads the ADC conversion value.
 * @param[out] adc_read_value.
 */
uint16_t adc_read(void) {
  adc_set_sample_time(ADC1, ADC_CHANNEL0, ADC_SMPR_SMP_239DOT5CYC);
    adc_set_regular_sequence(ADC1, 1, ADC_CHANNEL0);
    adc_start_conversion_direct(ADC1);
    while ( !adc_eoc(ADC1) ) {}
    return adc_read_regular(ADC1);
}

/**
 * Reads the ADC value and scales it up to volts.
 * @param[out] voltage.
 */
float adc_convert_voltage(uint16_t data) {
    float scale = 3.3/4096.0;
    return ((float)(data))*scale;
}
