#ifndef MICROPY_INCLUDED_RPI_PWM_H
#define MICROPY_INCLUDED_RPI_PWM_H

#include <stdint.h>
#include <stdbool.h>

void pwm_set_ctl(uint32_t id, uint32_t flags);
uint32_t pwm_get_ctl(uint32_t id);

void pwm_set_active(uint32_t id, bool active);
bool pwm_is_active(uint32_t id);

void pwm_set_period(uint32_t id, uint32_t period);
uint32_t pwm_get_period(uint32_t id);

void pwm_set_duty_ticks(uint32_t id, uint32_t duty_ticks);
uint32_t pwm_get_duty_ticks(uint32_t id);

bool pwm_fifo_available();
bool pwm_fifo_queue(uint32_t data);
void pwm_fifo_clear();
void pwm_fifo_set_active(bool active);
void pwm_err_clear();

#endif // MICROPY_INCLUDED_RPI_PWM_H
