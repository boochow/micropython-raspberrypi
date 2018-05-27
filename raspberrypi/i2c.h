#ifndef MICROPY_INCLUDED_RPI_I2C_H
#define MICROPY_INCLUDED_RPI_I2C_H

#include <stdint.h>
#include "bcm283x_i2c.h"

void i2c_init(i2c_t *i2c);
void i2c_deinit(i2c_t *i2c);
int i2c_write(i2c_t *i2c, const uint8_t *buf, const uint32_t buflen, bool stop);
int i2c_read(i2c_t *i2c, uint8_t *buf, const uint32_t readlen);

void i2c_clear_fifo(i2c_t *i2c);
#define i2c_set_slave(i2c, addr)    ((i2c)->A = (addr) & 0x7FU)
#define i2c_get_slave(i2c)          ((i2c)->A & 0x7FU)
void i2c_set_clock_speed(i2c_t *i2c, uint32_t speed);
uint32_t i2c_get_clock_speed(i2c_t *i2c);
int i2c_busy(i2c_t *i2c);

#endif // MICROPY_INCLUDED_RPI_I2C_H
