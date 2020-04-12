#ifndef UC_I2C_H_   /* Include guard */
#define UC_I2C_H_

#include <stdint.h>

enum I2C_RW {
	Read = 1,
	Write = 0
};

void i2c_setup(void);
void i2c_start_addr(uint8_t addr,enum I2C_RW rw);
void i2c_write_restart(uint8_t byte,uint8_t addr);
uint8_t i2c_read(bool lastf);
void i2c_stop(void);
void i2c_write(uint8_t byte);



#endif // UC_I2C_H_