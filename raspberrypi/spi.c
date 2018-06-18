#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "py/mpconfig.h"
#include "py/mperrno.h"
#include "bcm283x_spi.h"
#include "rpi.h"
#include "spi.h"

void spi_init(spi_t *spi, int polarity, int phase) {
    // set GPIO2, GPIO3 to alternate function 0
    uint32_t reg = spi->CS & ~(CS_CS | CS_CPOL | CS_CPHA | CS_REN);
    if (polarity != 0) {
        reg |= CS_CPOL;
    }
    if (phase != 0) {
        reg |= CS_CPHA;
    }
    spi->CS = reg | CLEAR_TX | CLEAR_RX;
    spi->CLK = 833; // 250MHz/833 = 300KHz
}

void spi_deinit(spi_t *spi) {
    spi->CS = spi->CS & ~CS_TA;
}

void spi_chip_select(spi_t *spi, int cs) {
    spi->CS = (spi->CS & ~(CS_CS)) | (cs & 0x3U);
}

int spi_transfer(spi_t *spi, uint8_t *buf_tx, const uint32_t wlen, uint8_t *buf_rx, const uint32_t rlen, uint8_t padding) {
    int result;
    int len = (wlen > rlen) ? wlen : rlen;
    int txcnt = 0;
    int rxcnt = 0;

    spi->CS |= CLEAR_TX | CLEAR_RX;
    spi->CS = spi->CS | CS_TA;

    for(;;) {
        if (spi->CS & CS_TXD) {
            uint8_t data;
            if (txcnt < len) {
                if (txcnt < wlen) {
                    data = *buf_tx++;
                } else {
                    data = padding;
                }
                spi->FIFO = data;
                txcnt++;
            }
        }

        if (spi->CS & CS_RXD) {
            if (rxcnt < len) {
                uint8_t data = spi->FIFO;
                if (rxcnt < rlen) {
                    *buf_rx++ = data;
                }
                rxcnt++;
            }
        }

        if ((txcnt == len) && (rxcnt == len) && (spi->CS & CS_DONE)) {
            result = len;
            break;
        }
    }
    spi->CS = spi->CS & ~CS_TA;
    return result;
}

void spi_set_clock_speed(spi_t *spi, uint32_t speed) {
    uint32_t val;
    uint32_t clk;
    clk = rpi_freq_core();
    val = clk / speed;
    if (val * speed < clk) {
        val++;
    }
    spi->CLK = val;
}

uint32_t spi_get_clock_speed(spi_t *spi) {
    uint32_t val = spi->CLK;
    if (val < 2) {
        val = 65536;
    }
    return rpi_freq_core() / val;
}
