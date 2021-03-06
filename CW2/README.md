# Temp Sensor and UART reporting

This assignment is divided into 3 incremental parts:

1. Implement a system that senses temperature and turns on alerts when values are outside the threshold.

        a) The temperature can be sensed with an analog or digital module, but it must be converted to degrees Celsius in the blue pill.
        b) Temperature will be sensed every 0.1 seconds, and sensor reading will be checked through interruptions.
        c) If the temperature sensed is greater than MaxTempTh, a LED will be turned on (MaxTempLED). If temperature sensed is lower than MinTempTh, another LED will be turned on (MinTempLED).
        d) The values for MaxTempTh and MinTempTh are arbitrary. Suggestion: Set MaxTempTh/MinTempTh values to be easy to be reached without much effort e.g. MinTempTh=24 is better than MinTempTh=0.
2. Implement a system that loops back a character sent from a PC terminal (minicom/picocom/putty/hyperterminal) and displays it in the same terminal.

        a) This loop back will be done through a USB2UART interface connected between the PC and the blue pill board.
        b) The blue pill will then receive the character through UART Rx and send it back through UART Tx.
        c) UART Rx interruption will be used to receive the character and used to send the character i.e. the main while loop will be empty.
        d) UART protocol settings are arbitrary but the suggestion for it is to be:
            - Baud Rate: 115200
            - Data bits: 8
            - Stop bits: 1
            - No parity
            - No Flow Control
3. Implement the system of 1. and use 2. to send the sampled data through UART to a PC terminal, while being able to configure temperature thresholds through commands sent from the terminal

        a) Points 1.[a-c] must be implemented. This includes sampling temperature every 0.1 seconds. The values of MaxTempTh and MinTempTh can be initially set to an arbitrary value that may change by configuring it.
        b) In NORMAL mode, every 0.5 seconds the blue pill will send status data to the PC terminal through UART. The status data must include current temperature (last sampled temperature is fine), values for Max and Min Temp Thresholds, and whether or not the Max or Min Temperatures are reached currently. All the data shall be printed in 1 line. A sample message can be:
            - Temp: 25C, MaxTempTh: 28C, MinTempTh: 24C, MaxTemp: Off, MinTemp: Off
        c) Whenever a “config key” has been pressed, the program will enter CONFIG mode, where Max or Min Temp Thresholds will be configured. In this mode, all interruptions other than UART Rx must be disabled i.e. no sampling will be done, and no status will be sent to the UART Tx. One config key will represent the MaxTempTh and another the MinTempTh, a suggestion is to use “x” and “n” keys.
            - If the MaxTempTh key was pressed the terminal will prompt the user with a message like:
            - Set Max Temperature Threshold (C):
            - Then the user will input the value in degrees Celsius and press . Upon recognizing the key, the value will be stored as the new MaxTempTh.
            - If data entered is not valid e.g. 3az23, the blue pill will send an error message saying something like:
            - Error, invalid data was sent. Please send a valid temperature (integer numbers)
            - The same behaviour will happen for the MinTempTh

The deliverables are:

- 3 videos on youtube, one for each task, displaying all behaviors specified above.
- Code and Makefile for each of the tasks, uploaded in a zip file.