#include "system_common.h"

/**
 * Sets up the system clock frequency.
 */
void system_clock_setup(void) {
	rcc_clock_setup_in_hsi_out_24mhz();
}
