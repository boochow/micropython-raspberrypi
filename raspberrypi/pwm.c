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

void pwm_queue_fifo(uint32_t data) {
    pwm_t *pwm = (pwm_t *)(PWM);
    while(pwm->STA & STA_FULL1) {
    }
    pwm->FIF1 = data;
}

void pwm_clear_fifo() {
    pwm_t *pwm = (pwm_t *)(PWM);
    pwm_set_ctl(0, pwm->CTL | CTL_CLRF1);
}

