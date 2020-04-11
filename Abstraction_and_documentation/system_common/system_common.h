#ifndef SYSTEM_COMMON_H_   /* Include guard */
#define SYSTEM_COMMON_H_

#include <libopencm3/stm32/rcc.h>

#define F_CLK 24000000

void system_clock_setup(void);

#endif // SYSTEM_COMMON_H_