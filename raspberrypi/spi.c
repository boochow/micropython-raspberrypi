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

int spi_transfer(spi_t *spi, uint8_t *buf_tx, const uint32_t wlen, uint8_t *buf_rx, const uint32_t rlen) {
    int result;
    int txlen = wlen;
    int rxlen = rlen;

    spi->CS = spi->CS | CS_TA;

    for(;;) {
        if (txlen > 0) {
            if (spi->CS & CS_TXD) {
                spi->FIFO = *buf_tx++;
                txlen--;
            }
        }

        if (rxlen > 0) {
            if (spi->CS & CS_RXD) {
                *buf_rx++ = spi->FIFO;
                rxlen--;
            }
        } else {
            while (spi->CS & CS_RXR) {
                spi->FIFO;
            }
        }

        if (spi->CS & CS_DONE) {
            if (wlen == 0) {
                result = rlen - rxlen;
            } else {
                result = wlen - txlen;
            }
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
