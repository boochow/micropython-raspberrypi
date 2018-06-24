#include <stdint.h>
#include "bcm283x_gpio.h"

void gpio_set_mode(uint32_t pin, uint32_t mode) {
    uint32_t reg;
    uint32_t pos;

    reg = GPFSEL0 + pin / 10 * 4;
    pos = (pin % 10) * 3;
    mode = mode & 7U;
    IOREG(reg) = (IOREG(reg) & ~(7 << pos)) | (mode << pos);
}

uint32_t gpio_get_mode(uint32_t pin) {
    uint32_t reg;
    uint32_t pos;

    reg = GPFSEL0 + pin / 10 * 4;
    pos = (pin % 10) * 3;
    return (IOREG(reg) >> pos) & 7U;
}

void gpio_set_level(uint32_t pin, uint32_t level) {
    uint32_t reg;

    reg = (pin > 31) ? 4 : 0;
    reg += (level == 0) ? GPCLR0 : GPSET0;
    IOREG(reg) = 1 << (pin & 0x1F);
}

uint32_t gpio_get_level(uint32_t pin) {
    uint32_t reg;

    reg = GPLEV0 + ((pin > 31) ? 4 : 0);
    if (IOREG(reg) & (1 << (pin & 0x1f))) {
        return 1;
    } else {
        return 0;
    }
}

static inline void delay_cycles(int32_t count)
{
    __asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
                 : "=r"(count): [count]"0"(count) : "cc");
}

void gpio_set_pull_mode(uint32_t pin, uint32_t pud) {
    uint32_t reg;

    IOREG(GPPUD) = pud;
    delay_cycles(150);
    reg = GPPUDCLK0 + (pin >> 5) * 4;
    IOREG(reg) = 1 << (pin & 0x1f);
    delay_cycles(150);
    IOREG(GPPUD) = 0;
    IOREG(reg) = 0;
}

