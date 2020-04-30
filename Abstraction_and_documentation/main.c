//  Copyright 2020 Copyright Equipo 2
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include "system_common/system_common.h"
#include "dimmer/dimmer.h"
#include "uc_timer/uc_timer.h"
#include "uc_interrupt/uc_interrupt.h"
#include "delay/delay.h"
#include "temp_sensor/temp_sensor.h"
#include "uc_uart/uc_uart.h"
#include "uc_usb/uc_usb.h"
#include <stdlib.h>
#include "LCD/LCD.h"

#define TIMER_PRESCALER        F_CLK / 10000  //7200 // TIM3 frequency =
                                            // 24Mhz/24K = 1khz
                                            //f = 10k
#define TIMER_PERIOD            1000  // 100ms
#define TIMER_CLOCK            RCC_TIM3
#define TIMER                  TIM3
#define TIMER_INTERRUPT        NVIC_TIM3_IRQ
#define TIMER_INTERRUPT_ISR    ISR_TIM3


#define CMD_MAX_TEMP         'x'
#define CMD_MIN_TEMP         'n'

#define CMD_DOWN_THR_MIN     's'
#define CMD_UP_THR_MIN         'w'
#define CMD_DOWN_THR_MAX     'a'
#define CMD_UP_THR_MAX         'd'
#define CMD_UP_DIM             'c'
#define CMD_DOWN_DIM         'z'
#define CMD_MENU             'm'

#define THR_STEP 1
#define DIM_STEP 10


#define MAX_DIM_LIMIT 100
#define MIN_DIM_LIMIT 0

#define MAX_THR_LIMIT 50
#define MIN_THR_LIMIT 0


#define VALUE_STR_LENGTH 11

static void gpio_setup(void);
static void timer_config(void);
static void uart_config(void);
void timer_interrupt(void);
void uart_interrupt(char chr);


typedef enum  {
  SEND_INFO,
  WAIT_INPUT
} globalSMType;

typedef enum  {
  WAIT_CONFIG_COMMAND,
  WAIT_VALUE,
  WAIT_MENU_NUMBER
} rx_SMType;

typedef enum  {
  MAX_THR,
  MIN_THR,
  DIM_VALUE
} thr_Type;

thr_Type threshold;

globalSMType global_state = SEND_INFO;
rx_SMType rx_state = WAIT_CONFIG_COMMAND;

char * error_msg = "\n\rError,  invalid data was sent. Please send a valid "
                   "temperature (integer numbers)\n\r\n\r";
char * big_int_msg = "\n\rThis number is bigger than the "
                     "expected values\n\r\n\r";
char * thr_error_msg = "\n\rPlease enter a number between %d and %d\n\r\n\r";
char * dim_error_msg = "\n\rPlease enter a number between %d and %d\n\r\n\r";
char * max_thr_msg = "Set Max Temperature Threshold (C): ";
char * min_thr_msg = "Set Min Temperature Threshold (C): ";
char * dim_msg = "Set Dimming Percentage (%%): ";

typedef enum  {
  ON,
  OFF,
} led_statusType;

char * led_status[2] =  {"On",  "Off"};

led_statusType min_led = OFF;
led_statusType max_led = OFF;

int MaxTempTh = 35;
int MinTempTh = 25;

int dimPercentage = 50;

int count = 0;
int value_count = 0;

char temp_value[VALUE_STR_LENGTH];

#define SLAVE_ADDRESS_LCD 0x27 //0x24 or 0x27



int main(void)  {
    system_clock_setup();
    gpio_setup();
    delay_setup();
    dimmer_setup();
    dimmer_update_percentage(dimPercentage);
    uart_config();
    temp_sensor_setup();

    //usb_start();

    LCD_Init(SLAVE_ADDRESS_LCD);

    LCD_Set_Cursor(1, 1);
    LCD_printf("Initializing");
    LCD_Set_Cursor(2, 1);
    LCD_printf("Temp sensor");
    delay_ms(1000);
    LCD_CMD(LCD_CLEAR);

    timer_config();

    usb_start();

    for (;;)  {


    }

    return 0;
}


static void gpio_setup(void)  {
     /* Enable GPIOC clock. */
    rcc_periph_clock_enable(RCC_GPIOC);

     /* Set GPIO8 (in GPIO port C) to 'output push-pull'. */
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
              GPIO_CNF_OUTPUT_PUSHPULL, GPIO13);
    gpio_set(GPIOC, GPIO13);  // start with led off

     // rcc_periph_clock_enable(RCC_GPIOA);         // Need GPIOA clock
     // gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ,
     //           GPIO_CNF_OUTPUT_PUSHPULL, GPIO1);
     // gpio_set(GPIOA, GPIO1);  //start with led off

    rcc_periph_clock_enable(RCC_GPIOB);         // Need GPIOA clock
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
              GPIO_CNF_OUTPUT_PUSHPULL, GPIO8);

    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
              GPIO_CNF_OUTPUT_PUSHPULL, GPIO5);
    gpio_set(GPIOB, GPIO5);  // start with led off
    gpio_set(GPIOB, GPIO8);  // start with led off
}


static void timer_config(void)  {
    uc_timer_setup(TIMER_CLOCK,  TIMER,  TIMER_PRESCALER);
    uc_timer_config_period(TIMER,  TIMER_PERIOD);
    uc_timer_enable_interrupt(TIMER,  TIMER_INTERRUPT);
    uc_timer_start(TIMER);

    uc_interrupt_attatch_function(TIMER_INTERRUPT_ISR,  timer_interrupt);
}

static void uart_config(void) {
    uart_pin_setup();
    uart_setup();
    uart_enable_rx_interrupt();
    uart_start();
    uc_interrupt_attatch_uart_function(uart_interrupt);
}



void timer_interrupt(void)  {
    int  temp;
    temp = temp_sensor_read();


    
    //usb_putc('x');

    if (temp > MaxTempTh) {
        gpio_clear(GPIOB, GPIO8);  // on
        max_led = ON;
    } else {
        gpio_set(GPIOB, GPIO8);  // off
        max_led = OFF;
    }
    if (temp < MinTempTh) {
        gpio_clear(GPIOB, GPIO5);  // on
        min_led = ON;
    } else {
        gpio_set(GPIOB, GPIO5);  // off
        min_led = OFF;
    }

    switch (global_state) {
        case SEND_INFO:
            if (count%5 == 0) {
                uart_printf("Temp: %dC,  MaxTempTh: %dC,  MinTempTh: %dC,  "
                "MaxTemp: %s,  MinTemp: %s,  Led Intensity: %d%%\n\r"
                            ,  temp,  MaxTempTh,  MinTempTh,  \
                            led_status[max_led], led_status[min_led],  \
                            dimPercentage);
                            
                usb_printf("Temp: %dC,  MaxTempTh: %dC,  MinTempTh: %dC\r\n"
                            ,  temp,  MaxTempTh,  MinTempTh);

                LCD_Set_Cursor(1, 1);
                LCD_printf("Temp: %d%cC", temp, 223);
                LCD_Set_Cursor(2, 1);
                LCD_printf("LED dimm: %d%%", dimPercentage);

            }
            break;
        case WAIT_INPUT:
            break;
        default:
            global_state = SEND_INFO;
            break;
    }

    count++;
}

void uart_interrupt(char chr) {
    switch (rx_state) {
        case WAIT_CONFIG_COMMAND:
            uart_printf("%c\n\r", chr);
            switch (chr) {
                case CMD_MAX_TEMP:
                    uart_printf(max_thr_msg);
                    global_state = WAIT_INPUT;
                    rx_state = WAIT_VALUE;
                    threshold = MAX_THR;
                    break;
                case CMD_MIN_TEMP:
                    uart_printf(min_thr_msg);
                    global_state = WAIT_INPUT;
                    rx_state = WAIT_VALUE;
                    threshold = MIN_THR;
                    break;
                case CMD_UP_DIM:
                    dimPercentage += DIM_STEP;
                    if (dimPercentage > MAX_DIM_LIMIT) \
                    dimPercentage = MAX_DIM_LIMIT;
                    dimmer_update_percentage(dimPercentage);
                    break;
                case CMD_DOWN_DIM:
                    dimPercentage -= DIM_STEP;
                    if (dimPercentage < MIN_DIM_LIMIT) \
                    dimPercentage = MIN_DIM_LIMIT;
                    dimmer_update_percentage(dimPercentage);
                    break;
                case CMD_UP_THR_MAX:
                    if (MaxTempTh + 1 <= MAX_THR_LIMIT) MaxTempTh += THR_STEP;
                    break;
                case CMD_DOWN_THR_MAX:
                    if (MaxTempTh > MinTempTh + 1) MaxTempTh -= THR_STEP;
                    break;
                case CMD_UP_THR_MIN:
                    if (MinTempTh + 1 < MaxTempTh) MinTempTh += THR_STEP;
                    break;
                case CMD_DOWN_THR_MIN:
                    if (MinTempTh -1  >= MIN_THR_LIMIT) MinTempTh -= THR_STEP;
                    break;
                case CMD_MENU:
                    uart_printf("\n\rEnter the number of the parameter "
                    "you want to configure:\n\r%s%s%s\n\rParameter: ", \
                                "1 - Max Temperature Threshold\n\r", \
                                "2 - Min Temperature Threshold\n\r", \
                                "3 - Dimming Percentage\n\r");
                    global_state = WAIT_INPUT;
                    rx_state = WAIT_MENU_NUMBER;
                    break;
                default:
                    uart_printf("\n\rCommand not valid. Type 'x' to configure "
                    "max \n\rthreshold or 'n' to "
                    "configure min threshold\n\r\n\r");
                    break;
            }

            break;
        case WAIT_VALUE:
            if (chr == '\r') {
                if (value_count > 0) {
                    temp_value[value_count] = 0;
                    int val = atoi(temp_value);
                    value_count = 0;
                    uart_printf("%c\n\r", chr);
                    switch (threshold) {
                        case(MAX_THR):
                            if ((val > MAX_THR_LIMIT) || (val <= MinTempTh)) {
                                 // return error
                                uart_printf("\n\r");
                                uart_printf(thr_error_msg,  \
                                  MinTempTh + 1,  MAX_THR_LIMIT);
                                uart_printf(max_thr_msg);
                            } else {
                                MaxTempTh = val;
                                global_state = SEND_INFO;
                                rx_state = WAIT_CONFIG_COMMAND;
                                }
                            break;
                        case(MIN_THR):
                            if ((val < MIN_THR_LIMIT) || (val >= MaxTempTh)) {
                                 // return error
                                uart_printf("\n\r");
                                uart_printf(thr_error_msg,  MIN_THR_LIMIT,  \
                                  MaxTempTh - 1);
                                uart_printf(min_thr_msg);
                            } else {
                                MinTempTh = val;
                                global_state = SEND_INFO;
                                rx_state = WAIT_CONFIG_COMMAND;
                            }
                            break;
                        case(DIM_VALUE):
                            if ((val < MIN_DIM_LIMIT) || \
                            (val > MAX_DIM_LIMIT)) {
                                  // return error
                                uart_printf("\n\r");
                                uart_printf(dim_error_msg,  MIN_DIM_LIMIT,  \
                                  MAX_DIM_LIMIT);
                                uart_printf(dim_msg);
                            } else {
                                dimPercentage = val;
                                dimmer_update_percentage(dimPercentage);
                                global_state = SEND_INFO;
                                rx_state = WAIT_CONFIG_COMMAND;
                            }
                            break;
                        default:
                            break;
                    }
                } else {
                    uart_printf("%c\n\r", chr);
                    uart_printf(error_msg);
                    switch (threshold) {
                        case MAX_THR:
                            uart_printf(max_thr_msg);
                            break;
                        case MIN_THR:
                            uart_printf(min_thr_msg);
                            break;
                        case DIM_VALUE:
                            uart_printf(dim_msg);
                            break;
                        default:
                            break;
                    }
                }
            } else if ((chr >= '0') && (chr <= '9')) {
                if (value_count > VALUE_STR_LENGTH - 2) {
                    value_count = 0;
                    uart_printf("%c\n\r", chr);
                    uart_printf(big_int_msg);
                    switch (threshold) {
                        case MAX_THR:
                            uart_printf(max_thr_msg);
                            break;
                        case MIN_THR:
                            uart_printf(min_thr_msg);
                            break;
                        case DIM_VALUE:
                            uart_printf(dim_msg);
                            break;
                        default:
                            break;
                    }

                } else {  // is a number
                    uart_printf("%c", chr);
                    temp_value[value_count++] = chr;
                }
            } else {
                value_count = 0;
                uart_printf("%c\n\r", chr);
                uart_printf(error_msg);
                switch (threshold) {
                        case MAX_THR:
                            uart_printf(max_thr_msg);
                            break;
                        case MIN_THR:
                            uart_printf(min_thr_msg);
                            break;
                        case DIM_VALUE:
                            uart_printf(dim_msg);
                            break;
                        default:
                            break;
                    }
            }
            break;
        case WAIT_MENU_NUMBER:
            uart_printf("%c\r\n",  chr);
            switch  (chr) {
                case '1':
                    uart_printf(max_thr_msg);
                    global_state = WAIT_INPUT;
                    rx_state = WAIT_VALUE;
                    threshold = MAX_THR;
                    break;
                case '2':
                    uart_printf(min_thr_msg);
                    global_state = WAIT_INPUT;
                    rx_state = WAIT_VALUE;
                    threshold = MIN_THR;
                    break;
                case '3':
                    uart_printf(dim_msg);
                    global_state = WAIT_INPUT;
                    rx_state = WAIT_VALUE;
                    threshold = DIM_VALUE;
                    break;
                default:
                    uart_printf("Please select a number in the "
                    "list.\r\n%s%s%s\n\rParameter: ", \
                                "1 - Max Temperature Threshold\n\r", \
                                "2 - Min Temperature Threshold\n\r", \
                                "3 - Dimming Percentage\n\r");
                    break;
            }
            break;
        default:
            rx_state = WAIT_CONFIG_COMMAND;
            break;
    }
}
