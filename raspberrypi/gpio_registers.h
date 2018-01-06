#define IOREG(X)  (*(volatile uint32_t *) (X))
#define GPIO(X)   IOREG((X) + GPIO_BASE)

// GPIO registers

#define GPIO_BASE 0x20200000
#define GPFSEL0 0x00
#define GPFSEL1 0x04
#define GPFSEL2 0x08
#define GPFSEL3 0x0C
#define GPFSEL4 0x10
#define GPFSEL5 0x14
#define GPSET0  0x1C
#define GPSET1  0x20
#define GPCLR0  0x28
#define GPCLR1  0x2C
#define GPLEV0  0x34
#define GPLEV1  0x38
#define GPEDS0  0x40
#define GPEDS1  0x44
#define GPREN0  0x4C
#define GPREN1  0x50
#define GPFEN0  0x58
#define GPFEN1  0x5C
#define GPHEN0  0x64
#define GPHEN1  0x68
#define GPLEN0  0x70
#define GPLEN1  0x74
#define GPAREN0 0x7C
#define GPAREN1 0x80
#define GPAFEN0 0x88
#define GPAFEN1 0x8C
#define GPPUD   0x94
#define GPPUDCLK0 0x98
#define GPPUDCLK1 0x9C

// GPIO Alternate function selecter values
#define GPF_INPUT  0U
#define GPF_OUTPUT 1U
#define GPF_ALT_0  4U
#define GPF_ALT_1  5U
#define GPF_ALT_2  6U
#define GPF_ALT_3  7U
#define GPF_ALT_4  3U
#define GPF_ALT_5  2U
