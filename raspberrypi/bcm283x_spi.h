#ifndef MICROPY_INCLUDED_RPI_BCM283X_SPI_H
#define MICROPY_INCLUDED_RPI_BCM283X_SPI_H

#define SPI0 (IO_BASE + 0x204000)

typedef volatile struct _spi_t {
    uint32_t CS;
    uint32_t FIFO;
    uint32_t CLK;
    uint32_t DLEN;
    uint32_t LTOH;
    uint32_t DC;
} spi_t;

#define CS_LEN_LONG (1<<25)
#define CS_DMA_LEN  (1<<24)
#define CS_CSPOL2   (1<<23)
#define CS_CSPOL1   (1<<22)
#define CS_CSPOL0   (1<<21)
#define CS_RXF      (1<<20)
#define CS_RXR      (1<<19)
#define CS_TXD      (1<<18)
#define CS_RXD      (1<<17)
#define CS_DONE     (1<<16)
#define CS_TE_EN    (1<<15)
#define CS_LMONO    (1<<14)
#define CS_LEN      (1<<13)
#define CS_REN      (1<<12)
#define CS_ADCS     (1<<11)
#define CS_INTR     (1<<10)
#define CS_INTD     (1<<9)
#define CS_DMAEN    (1<<8)
#define CS_TA       (1<<7)
#define CS_CSPOL    (1<<6)
#define CS_CLEAR    (3<<4)
#define CS_CPOL     (1<<3)
#define CS_CPHA     (1<<2)
#define CS_CS       (3<<0)

#define CLEAR_TX    (1<<4)
#define CLEAR_RX    (2<<4)

#endif // MICROPY_INCLUDED_RPI_BCM283X_SPI_H
