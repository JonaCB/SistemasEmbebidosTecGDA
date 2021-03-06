## CW1 - Oscillating PWM
### Requirements for this assignment are:

Deliver a system that implements a light oscillating LED. The LED will be oscillating its intensity in two ways:

Light intensity will be oscillating in a sinusoidal pattern, in the range of 0% to 100% intensity
Every period of the sinusoidal wave generated, the frequency f of oscillation will be increased by 0.1 i.e.  f+=0.1f, with a max frequency of 10f, and min frequency f. When max frequency is reached, f starts decrementing by 0.1 i.e. f-=0.1f, until min frequency is reached, then the pattern starts again.
Set f to 1Hz, therefore 10f is 10Hz
In order to vary the intensity it is necessary to generate a PWM signal, in which the duty cycle maps to the light intensity. It is given then, that the LED must be connected to a Timer output (for which the PWM signal will be generated). 

There are 2 mini milestones: 

1. Generate the sine wave of any frequency (visible obviously), varying the intensity of the LED using PWM
2. Connect a LED to a Timer output, and toggle the LED with the following progression: On 1 sec, Off 1/1.1 secs, On 1/1.2 secs, Off 1/1.3 secs, .... until On 1/10 secs, then revert progression as: On 1/10 secs, Off 1/9.9 secs, On 1/9.8 secs, .... until On 1sec

Deliverables are:

- A video showing the code being compiled and downloaded to the STM32 board through the ST-Link programmer, and the light oscillating starting in 1Hz, going to 10Hz and coming back to 1Hz. The video shall be uploaded to youtube
- The source code and the Makefile to compile the two mini milestones and the final deliverable