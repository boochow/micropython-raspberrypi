#include <stdint.h>
#include <stdbool.h>
#include "py/mpconfig.h"
#include "bcm283x_i2c.h"
#include "rpi.h"
#include "i2c.h"

void i2c_init(i2c_t *i2c) {
    i2c->S |= S_CLKT | S_ERR | S_DONE;
    i2c->C |= C_I2CEN | C_CLEAR;
}

void i2c_deinit(i2c_t *i2c) {
    i2c->S |= S_CLKT | S_ERR | S_DONE;
    i2c->C |= C_CLEAR;
    i2c->C = i2c->C & ~C_I2CEN;
}

int i2c_write(i2c_t *i2c, const uint8_t *buf, const uint32_t buflen) {
    i2c->DLEN = buflen;
    i2c->S |= S_DONE | S_ERR | S_CLKT;
    i2c->C = i2c->C & ~C_READ;
    i2c->C |= C_CLEAR;
    i2c->C |= C_ST;
    int len = buflen;
    for(;;) {
        if (i2c->S & S_ERR) {
            // No Ack Error
            i2c->S |= S_ERR;
            return -1;
        } else if (i2c->S & S_CLKT) {
            // Timeout Error
            i2c->S |= S_CLKT;
            return -2;
        } else if (i2c->S & S_DONE) {
            // Transfer Done
            i2c->S |= S_DONE;
            return buflen - len;
        }

        if ((i2c->S & S_TXD) && (len != 0)){
            // FIFO has a room
            i2c->FIFO = *buf++;
            len--;
        }
    }
}

int i2c_read(i2c_t *i2c, uint8_t *buf, const uint32_t readlen) {
    int len = readlen;
    i2c->DLEN = readlen;
    i2c->S |= S_DONE | S_ERR | S_CLKT;
    i2c->C |= C_READ;
    i2c->C |= C_CLEAR;
    i2c->C |= C_ST;
    for(;;) {
        if (i2c->S & S_TA) {
            continue;
        } else if (i2c->S & S_ERR) {
            // No Ack Error
            i2c->S |= S_ERR;
            return -1;
        } else if (i2c->S & S_CLKT) {
            // Timeout Error
            i2c->S |= S_CLKT;
            return -2;
        } else if (i2c->S & S_DONE) {
            // Transfer Done
            i2c->S |= S_DONE;
            return readlen - len;
        }

        if (i2c->S & S_RXD) {
            uint8_t data = i2c->FIFO;
            if (len != 0) {
                *buf++ = data;
                len--;
            } else {
                return readlen - len;
            }
        }
    }
}

void i2c_flush(i2c_t *i2c) {
    i2c->C |= C_CLEAR;
}

void i2c_set_clock_speed(i2c_t *i2c, uint32_t speed) {
    uint32_t cdiv = rpi_freq_core() / speed;
    cdiv = (cdiv < 2) ? 2 : cdiv;
    cdiv = (cdiv > 0xfffe) ? 0xfffe : cdiv;
    uint32_t fedl = (cdiv > 15) ? (cdiv / 16) : 1;
    uint32_t redl = (cdiv > 3) ? (cdiv / 4) : 1;
    i2c->DIV = cdiv;
    i2c->DEL = (fedl << 16) | redl;
}

uint32_t i2c_get_clock_speed(i2c_t *i2c) {
    return rpi_freq_core() / i2c->DIV;
}

int i2c_busy(i2c_t *i2c) {
    return i2c->S & S_TA;
}
