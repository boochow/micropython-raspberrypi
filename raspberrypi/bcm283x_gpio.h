#ifndef MICROPY_INCLUDED_RPI_BCM283X_GPIO_H
#define MICROPY_INCLUDED_RPI_BCM283X_GPIO_H

#include "bcm283x.h"

#define GPIO_BASE (0x200000 + IO_BASE)
#define GPIO(X)   ((X) + GPIO_BASE)

// GPIO registers

#define GPFSEL0 GPIO(0x00)
#define GPFSEL1 GPIO(0x04)
#define GPFSEL2 GPIO(0x08)
#define GPFSEL3 GPIO(0x0C)
#define GPFSEL4 GPIO(0x10)
#define GPFSEL5 GPIO(0x14)
#define GPSET0  GPIO(0x1C)
#define GPSET1  GPIO(0x20)
#define GPCLR0  GPIO(0x28)
#define GPCLR1  GPIO(0x2C)
#define GPLEV0  GPIO(0x34)
#define GPLEV1  GPIO(0x38)
#define GPEDS0  GPIO(0x40)
#define GPEDS1  GPIO(0x44)
#define GPREN0  GPIO(0x4C)
#define GPREN1  GPIO(0x50)
#define GPFEN0  GPIO(0x58)
#define GPFEN1  GPIO(0x5C)
#define GPHEN0  GPIO(0x64)
#define GPHEN1  GPIO(0x68)
#define GPLEN0  GPIO(0x70)
#define GPLEN1  GPIO(0x74)
#define GPAREN0 GPIO(0x7C)
#define GPAREN1 GPIO(0x80)
#define GPAFEN0 GPIO(0x88)
#define GPAFEN1 GPIO(0x8C)
#define GPPUD   GPIO(0x94)
#define GPPUDCLK0 GPIO(0x98)
#define GPPUDCLK1 GPIO(0x9C)

// GPIO Alternate function selecter values
#define GPF_INPUT  0U
#define GPF_OUTPUT 1U
#define GPF_ALT_0  4U
#define GPF_ALT_1  5U
#define GPF_ALT_2  6U
#define GPF_ALT_3  7U
#define GPF_ALT_4  3U
#define GPF_ALT_5  2U

// GPIO Pull-Up/Pull-Down (GPPUD) values
#define GPPUD_NONE    0U
#define GPPUD_EN_UP   1U
#define GPPUD_EN_DOWN 2U

// set / get GPIO Alternate function
void gpio_set_mode(uint32_t pin, uint32_t mode);
uint32_t gpio_get_mode(uint32_t pin);

// set output mode GPIO pin to low or high
void gpio_set_level(uint32_t pin, uint32_t level);

// get input mode GPIO pin level (low or high)
uint32_t gpio_get_level(uint32_t pin);

// set input mode GPIO pin pull up mode (off or pull up or pull down)
void gpio_set_pull_mode(uint32_t pin, uint32_t pud);

#endif // MICROPY_INCLUDED_RPI_BCM283X_GPIO_H
