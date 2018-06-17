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
    int txlen = wlen;
    int rxlen = rlen;
    int len = (wlen > rlen) ? wlen : rlen;
    int cnt = len;

    spi->CS |= CLEAR_TX | CLEAR_RX;
    spi->CS = spi->CS | CS_TA;

    for(;;) {
        if (cnt == 0) {
            if (spi->CS & CS_DONE) {
                result = len;
                break;
            }
            continue;
        }
        
        if (spi->CS & CS_TXD) {
            uint8_t data;
            if (txlen > 0) {
                data = *buf_tx++;
                txlen--;
            } else {
                data = padding;
            }
            spi->FIFO = data;
        }

        if (spi->CS & CS_RXD) {
            uint8_t data = spi->FIFO;
            if (rxlen > 0) {
                *buf_rx++ = data;
                rxlen--;
            }
            cnt--;
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
