#ifndef MICROPY_INCLUDED_RPI_SPI_H
#define MICROPY_INCLUDED_RPI_SPI_H

#include <stdint.h>
#include "bcm283x_spi.h"

void spi_init(spi_t *spi, int polarity, int phase);
void spi_deinit(spi_t *spi);

void spi_set_clock_speed(spi_t *spi, uint32_t speed);
uint32_t spi_get_clock_speed(spi_t *spi);

void spi_chip_select(spi_t *spi, int cs);

int spi_transfer(spi_t *spi, uint8_t *buf_tx, const uint32_t wlen, uint8_t *buf_rx, const uint32_t rlen, uint8_t padding);

#endif // MICROPY_INCLUDED_RPI_SPI_H
