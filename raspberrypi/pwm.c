#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bcm283x_pwm.h"
#include "pwm.h"

void pwm_set_ctl(uint32_t id, uint32_t flags) {
    if (id < 2) {
        uint32_t mask = ~(0xff << id * 8);
        pwm_t *pwm = (pwm_t *)(PWM);
        pwm->CTL = (pwm->CTL & mask) | flags << id * 8;
    }
}

uint32_t pwm_get_ctl(uint32_t id) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        return pwm->CTL & 0xff;
    } else {
        return (pwm->CTL >> 8) & 0xff;
    }
}

bool pwm_is_active(uint32_t id) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        return ((pwm->CTL & CTL_PWEN1) > 0);
    } else if (id == 1) {
        return ((pwm->CTL & CTL_PWEN2) > 0);
    }
    return false;
}

void pwm_set_active(uint32_t id, bool active) {
    if (id < 2) {
        pwm_t *pwm = (pwm_t *)(PWM);
        if (active) {
            pwm->CTL = pwm->CTL | CTL_PWEN1 << id * 8;
        } else {
            uint32_t mask = ~(CTL_PWEN1 << id * 8);
            pwm->CTL = pwm->CTL & mask;
        }
    }
}

void pwm_fifo_set_active(bool active) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (active) {
        pwm->CTL |= CTL_PWEN1 | CTL_PWEN2;
    } else {
        uint32_t mask = ~(CTL_PWEN1 | CTL_PWEN2);
        pwm->CTL &= mask;
    }
}

uint32_t pwm_get_period(uint32_t id) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        return pwm->RNG1;
    } else {
        return pwm->RNG2;
    }
}

uint32_t pwm_get_duty_ticks(uint32_t id) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        return pwm->DAT1;
    } else {
        return pwm->DAT2;
    }
}

void pwm_set_period(uint32_t id, uint32_t period) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        pwm->RNG1 = period;
    } else {
        pwm->RNG2 = period;
    }
}

void pwm_set_duty_ticks(uint32_t id, uint32_t duty_ticks) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        pwm->DAT1 = duty_ticks;
    } else {
        pwm->DAT2 = duty_ticks;
    }
}

bool pwm_fifo_available() {
    pwm_t *pwm = (pwm_t *)(PWM);
    return ((pwm->STA & STA_FULL1) == 0);
}

bool pwm_fifo_queue(uint32_t data) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (pwm->STA & STA_FULL1) {
        return false;
    } else {
        pwm->FIF1 = data;
        if (pwm->STA & STA_WERR1) {
            pwm->STA |= STA_WERR1;
        }
        return true;
    }
}

void pwm_fifo_clear() {
    pwm_t *pwm = (pwm_t *)(PWM);
    pwm_set_ctl(0, pwm->CTL | CTL_CLRF1);
}

void pwm_err_clear() {
    pwm_t *pwm = (pwm_t *)(PWM);
    while (pwm->STA & STA_WERR1) {
        pwm->STA |= STA_WERR1;
    }
    while (pwm->STA & STA_RERR1) {
        pwm->STA |= STA_RERR1;
    }
    while (pwm->STA & STA_GAPO1) {
        pwm->STA |= STA_GAPO1;
    }
    while (pwm->STA & STA_GAPO2) {
        pwm->STA |= STA_GAPO2;
    }
    while (pwm->STA & STA_BERR) {
        pwm->STA |= STA_BERR;
    }
}
