#include <stdint.h>
#include <stdbool.h>
#include "py/mpconfig.h"
#include "py/mperrno.h"
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

void i2c_start(i2c_t *i2c) {
    i2c->C |= C_ST;
}

/*
  i2c_abort_write(i2c) is used for generating pseudo stop condition.
  Since bcm2835 I2C controller automatically generates stop condition
  when the amount of transfer reaches to DLEN, it is impossible to
  determine the timing of generating stop condition AFTER a transfer
  started. So I made this function to "break" on-going transfer and
  generate stop condition whenever it is necessary. The transfer used
  with this function should set DLEN large enough to avoid automatic
  stop condition generation.
*/
void i2c_abort_write(i2c_t *i2c) {
    while (!(i2c->S & S_TXE));
    i2c_clear_fifo(i2c);
}

int i2c_busy(i2c_t *i2c) {
    return (i2c->S & S_TA) && ((i2c->S & S_DONE) == 0);
}

void i2c_clear_fifo(i2c_t *i2c) {
    i2c->C |= C_CLEAR;
}

int i2c_result(i2c_t *i2c) {
    if (i2c->S & S_ERR) {
        // No Ack Error
        i2c->S |= S_ERR;
        return -MP_EIO;
    } else if (i2c->S & S_CLKT) {
        // Timeout Error
        i2c->S |= S_CLKT;
        return -MP_ETIMEDOUT;
    } else if (i2c->S & S_DONE) {
        // Transfer Done
        i2c->S |= S_DONE;
    }
    return 0;
}

int i2c_write(i2c_t *i2c, const uint8_t *buf, const uint32_t buflen, bool stop) {
    int len = buflen;
    bool continued = false;

    if ((i2c->S & S_TA) && ((i2c->C & C_READ) == 0)){
        // previous write transfer is still active (maybe stop=False)
        continued = true;
    } else {
        // start new transfer
        i2c->S |= S_DONE | S_ERR | S_CLKT;
        i2c->C = i2c->C & ~C_READ;
        while ((i2c->S & S_TXD) && (len != 0)){
            i2c->FIFO = *buf++;
            --len;
        }
        if (stop) {
            i2c->DLEN = buflen;
        } else {
            // do not STOP after sending all data in buf; see i2c_abort_write()
            i2c->DLEN = 0xffff;
        }
        i2c_start(i2c);
        do { } while (!(i2c->S & S_TA));
    }
    for(;;) {
        if (i2c->S & (S_ERR | S_CLKT)) {
            // error occured
            return i2c_result(i2c);
        } else if (stop && (i2c->S & S_DONE)) {
            // transfer success
            return buflen - len;
        } else if (!stop && (len == 0)) {
            // maybe transfer success
            return buflen - len;
        }

        if (continued && (len == 0)) {
            if (stop) {
                i2c_abort_write(i2c);
                while (!(i2c->S & S_DONE));
            }
            return buflen;
        }

        while ((i2c->S & S_TXD) && (len != 0)) {
            i2c->FIFO = *buf++;
            --len;
        }
    }
}

int i2c_read(i2c_t *i2c, uint8_t *buf, const uint32_t readlen) {
    if ((i2c->S & S_TA) && ((i2c->C & C_READ) == 0)){
        // previous write transfer is still active
        i2c_abort_write(i2c);
    } else {
        i2c->S |= S_DONE | S_ERR | S_CLKT;
    }
    i2c->DLEN = readlen;
    i2c->C |= C_READ;
    i2c_start(i2c);
    int len = readlen;
    for(;;) {
        if (i2c->S & (S_ERR | S_CLKT)) {
            // error occured
            return i2c_result(i2c);
        } else if (i2c->S & S_DONE) {
            // transfer success
            return readlen - len;
        }

        if (i2c->S & S_RXD) {
            uint8_t data = i2c->FIFO & 0xffU;
            if (len != 0) {
                *buf++ = data;
                len--;
            }
        }
    }
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
    uint32_t val = i2c->DIV & 0x7fff;
    if (val == 0) {
        val = 32768;
    }
    return rpi_freq_core() / val;
}
