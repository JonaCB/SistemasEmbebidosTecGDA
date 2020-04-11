#include "system_common.h"

void system_clock_setup(void) {
	
	rcc_clock_setup_in_hsi_out_24mhz();

}