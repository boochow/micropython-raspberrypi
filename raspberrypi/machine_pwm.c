#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "modmachine.h"
#include "rpi.h"
#include "bcm283x_gpio.h"
#include "bcm283x_pwm.h"
#include "pwm.h"
#include "machine_pin_util.h"

#define DEFAULT_PERIOD  960
#define DEFAULT_TICK_HZ 960000
#define DEFAULT_TIMEOUT 1000

typedef struct _machine_pwm_obj_t {
    const mp_obj_base_t base;
    const uint32_t id;
    int32_t pin;
    uint32_t tick_hz;
} machine_pwm_obj_t;

static machine_pwm_obj_t machine_pwm_obj[] = {
    {{&machine_pwm_type}, 0, -1, 0},
    {{&machine_pwm_type}, 1, -1, 0},
};

static int pwm_select_pin(const int pin) {
    static const int pwm_alt[3][9] = {
        {12, 13, 18, 19, 40, 41, 45, 52, 53},
        { 0,  1,  0,  1,  0,  1,  1,  0,  1},
        { GPF_ALT_0, GPF_ALT_0, GPF_ALT_5, GPF_ALT_5, \
          GPF_ALT_0, GPF_ALT_0, GPF_ALT_0, GPF_ALT_1, GPF_ALT_1}
    };
    int i;
    for(i = 0; i < 9; i++) {
        if (pwm_alt[0][i] == pin) {
            break;
        }
    }
    if (i < 9) {
        gpio_set_mode(pin, pwm_alt[2][i]);
        return pwm_alt[1][i];
    } else {
        return -1;
    }
}

STATIC mp_obj_t machine_pwm_init_helper(machine_pwm_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_tick_hz, ARG_period, ARG_duty_ticks, ARG_freq, ARG_duty_u16, ARG_active, ARG_ms, ARG_mode, ARG_polarity, ARG_silence, ARG_use_fifo, ARG_fifo_repeat };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_tick_hz,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_period,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_duty_ticks,  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_freq,        MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_duty_u16,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_active,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_ms,          MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_mode,        MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_polarity,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_silence,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_use_fifo,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_fifo_repeat, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];

    // parse args
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // initialize PWM controller
    uint32_t old;
    old = pwm_get_ctl(self->id);
    uint32_t flags = 0;

    switch(args[ARG_active].u_int){
    case 0:
        break;
    case 1:
        flags |= CTL_PWEN1;
        break;
    default:
        flags |= old & CTL_PWEN1;
    }

    switch (args[ARG_ms].u_int) {
    case 0:
        break;
    case 1:
        flags |= CTL_MSEN1;
        break;
    default:
        flags |= old & CTL_MSEN1;
    }
    switch (args[ARG_mode].u_int) {
    case 0:
        break;
    case 1:
        flags |= CTL_MODE1;
        break;
    default:
        flags |= old & CTL_MODE1;
    }
    switch (args[ARG_polarity].u_int) {
    case 0:
        break;
    case 1:
        flags |= CTL_POLA1;
        break;
    default:
        flags |= old & CTL_POLA1;
    }
    switch (args[ARG_silence].u_int) {
    case 0:
        break;
    case 1:
        flags |= CTL_SBIT1;
        break;
    default:
        flags |= old & CTL_SBIT1;
    }
    switch (args[ARG_use_fifo].u_int) {
    case 0:
        break;
    case 1:
        flags |= CTL_USEF1;
        break;
    default:
        flags |= old & CTL_USEF1;
    }
    switch (args[ARG_fifo_repeat].u_int) {
    case 0:
        break;
    case 1:
        flags |= CTL_RPTL1;
        break;
    default:
        flags |= old & CTL_RPTL1;
    }
    pwm_set_ctl(self->id, flags);

    pwm_err_clear();

    // set the instance property tick_hz
    if (args[ARG_tick_hz].u_int > 0) {
        self->tick_hz = args[ARG_tick_hz].u_int;
    } else if ((args[ARG_freq].u_int > 0) && (args[ARG_period].u_int > 0)){
        self->tick_hz = args[ARG_freq].u_int * args[ARG_period].u_int;
    } else {
        self->tick_hz = DEFAULT_TICK_HZ;
    }

    // set period and duty_ticks
    uint32_t period;
    uint64_t duty_ticks;

    if (args[ARG_period].u_int > 0) {
        period = args[ARG_period].u_int;
    } else if (args[ARG_freq].u_int > 0) {
        period = self->tick_hz / args[ARG_freq].u_int;
    } else {
        period = DEFAULT_PERIOD;
    }
    pwm_set_period(self->id, period);

    if (args[ARG_duty_ticks].u_int > 0) {
        duty_ticks = args[ARG_duty_ticks].u_int;
    } else if (args[ARG_duty_u16].u_int > 0) {
        duty_ticks = (self->tick_hz * args[ARG_duty_u16].u_int) >> 16;
    } else {
        duty_ticks = period >> 1;
    }
    pwm_set_duty_ticks(self->id, duty_ticks & 0xffffffff);

    return mp_const_none;
}

STATIC mp_obj_t machine_pwm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // first argment is PWM number or Pin object
    int id = -1;
    int pin_num = -1;
    // set up GPIO alternate function and get PWM id if the pin is specified
    if (mp_obj_get_type(all_args[0]) == &machine_pin_type) {
        pin_num = pin_get_id(all_args[0]);
        if (pin_num < 0) {
            mp_raise_ValueError("invalid pin");
        } else {
            id = pwm_select_pin(pin_num);
            if (id < 0) {
                mp_raise_ValueError("invalid pin");
            }
        }
    // or use PWM id without specifing GPIO pin (pin_num = -1)
    } else if (MP_OBJ_IS_INT(all_args[0])) {
        id = mp_obj_get_int(all_args[0]);
        if ((id < 0) || (id > 1)) {
            mp_raise_ValueError("invalid PWM number");
        }    
    }

    machine_pwm_obj_t *self = (machine_pwm_obj_t*) &machine_pwm_obj[id];
    self->pin = pin_num;

    // parse keyword arguments
    mp_map_t kw_args;
    mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
    machine_pwm_init_helper(self, n_args - 1, all_args + 1, &kw_args);

    return self;
}

STATIC mp_obj_t machine_pwm_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_pwm_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pwm_init_obj, 0, machine_pwm_init);

STATIC void machine_pwm_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pwm_obj_t *self = self_in;
    mp_printf(print, "PWM(");
    if (self->pin >= 0) {
        mp_printf(print, "Pin(%u)", self->pin);
    } else {
        mp_printf(print, "%u", self->id);
    }

    mp_printf(print, ", tick_hz=%u", self->tick_hz);
    mp_printf(print, ", period=%u", pwm_get_period(self->id));
    mp_printf(print, ", duty_ticks=%u", pwm_get_duty_ticks(self->id));

    uint32_t flags = pwm_get_ctl(self->id);
    if (flags & CTL_PWEN1) {
        mp_printf(print, ", active=True");
    } else {
        mp_printf(print, ", active=False");
    }
    qstr mode_qstr = MP_QSTR_MS_DISABLE;
    if (flags & CTL_MSEN1) {
        mode_qstr = MP_QSTR_MS_ENABLE;
    }
    mp_printf(print, ", ms=PWM.%s", qstr_str(mode_qstr));

    mode_qstr = MP_QSTR_MODE_PWM;
    if (flags & CTL_MODE1) {
        mode_qstr = MP_QSTR_MODE_SERIALIZER;
    }
    mp_printf(print, ", mode=PWM.%s", qstr_str(mode_qstr));
    mp_printf(print, ", polarity=%u", (flags & CTL_POLA1) ? 1 : 0);
    mp_printf(print, ", silence=%u", (flags & CTL_SBIT1) ? 1 : 0);
    mp_printf(print, ", use_fifo=%u", (flags & CTL_USEF1) ? 1 : 0);
    mp_printf(print, ", fifo_repeat=%u)", (flags & CTL_RPTL1) ? 1 : 0);
}

STATIC mp_obj_t machine_pwm_deinit(mp_obj_t self_in) {
    machine_pwm_obj_t *self = self_in;
    pwm_set_active(self->id, false);
    self->pin = -1;
    self->tick_hz = 0;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pwm_deinit_obj, machine_pwm_deinit);

/// \method period(value)
/// Get or set the pwm period.
STATIC mp_obj_t machine_pwm_period(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        return MP_OBJ_NEW_SMALL_INT(pwm_get_period(self->id));
    } else {
        // set
        pwm_set_period(self->id, mp_obj_get_int(args[1]));
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_period_obj, 1, 2, machine_pwm_period);

/// \method duty_ticks(value)
/// Get or set the pwm duty_ticks.
STATIC mp_obj_t machine_pwm_duty_ticks(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        return MP_OBJ_NEW_SMALL_INT(pwm_get_duty_ticks(self->id));
    } else {
        // set
        pwm_set_duty_ticks(self->id, mp_obj_get_int(args[1]));
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_duty_ticks_obj, 1, 2, machine_pwm_duty_ticks);

/// \method duty_u16(value)
/// Set the duty cycle as a ratio duty_u16 / 2^16.
STATIC mp_obj_t machine_pwm_duty_u16(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        uint64_t duty = pwm_get_duty_ticks(self->id) << 16;
        return MP_OBJ_NEW_SMALL_INT(duty / pwm_get_period(self->id));
    } else {
        // set
        uint64_t duty = pwm_get_period(self->id);
        duty = duty * mp_obj_get_int(args[1]);
        pwm_set_duty_ticks(self->id, (duty >> 16) & 0xffffffffU);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_duty_u16_obj, 1, 2, machine_pwm_duty_u16);

/// \method active([value])
/// activate or deactivate the pwm output.
STATIC mp_obj_t machine_pwm_active(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        if (pwm_is_active(self->id)) {
            return mp_const_true;
        } else {
            return mp_const_false;
        }
    } else {
        // set
        pwm_set_active(self->id, mp_obj_get_int(args[1]));
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_active_obj, 1, 2, machine_pwm_active);

/// \method fifo_active([value])
/// activate or deactivate all of pwm outputs.
STATIC mp_obj_t machine_pwm_fifo_active(mp_obj_t self_in, mp_obj_t active) {
    pwm_fifo_set_active(mp_obj_get_int(active));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_pwm_fifo_active_obj, machine_pwm_fifo_active);

/// \method freq(value)
/// Set the frequency in Hz
STATIC mp_obj_t machine_pwm_freq(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        return MP_OBJ_NEW_SMALL_INT(self->tick_hz / pwm_get_period(self->id));
    } else {
        // set
        uint32_t period = self->tick_hz / mp_obj_get_int(args[1]);
        uint64_t duty = pwm_get_duty_ticks(self->id);
        duty = (duty << 32) / pwm_get_period(self->id) * period;
        pwm_set_duty_ticks(self->id, (duty >> 32) & 0xffffffffU);
        pwm_set_period(self->id, period);
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_freq_obj, 1, 2, machine_pwm_freq);

/// \method fifo_write(buf1[, buf2])
/// copy buffer(s) to FIFO
STATIC mp_obj_t machine_pwm_fifo_write(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_buf1, ARG_buf2, ARG_timeout };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buf1, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_buf2, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, 
          {.u_int = DEFAULT_TIMEOUT} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    // parse args
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    if ((args[ARG_buf1].u_obj == mp_const_none) && 
        (args[ARG_buf2].u_obj == mp_const_none)) {
        return mp_obj_new_bool(pwm_fifo_available());
    }

    uint64_t t_start;
    t_start = systime();
    mp_buffer_info_t bufinfo1, bufinfo2;
    size_t pos = 0;

    if (args[ARG_buf1].u_obj != mp_const_none) {
        uint64_t timeout = args[ARG_timeout].u_int;
        mp_get_buffer_raise(args[ARG_buf1].u_obj, &bufinfo1, MP_BUFFER_READ);
        if (args[ARG_buf2].u_obj == mp_const_none) {
            // write from single buffer
            uint32_t *p = (uint32_t *) bufinfo1.buf;
            for (pos = 0; pos < bufinfo1.len >> 2; pos++) {
                if (pwm_fifo_available()) {
                    if (pwm_fifo_queue(*p)) {
                        p++;
                    }
                }
                if (elapsed_from(t_start) > timeout) {
                    mp_raise_OSError(MP_EBUSY);
                }
            }
        } else {
            // write from two buffers
            mp_get_buffer_raise(args[ARG_buf2].u_obj, &bufinfo2, MP_BUFFER_READ);
            uint32_t maxpos, max1, max2;
            max1 = bufinfo1.len >> 2;
            max2 = bufinfo2.len >> 2;
            maxpos = max1 > max2 ? max1 : max2;

            uint32_t *p1 = (uint32_t *) bufinfo1.buf;
            uint32_t *p2 = (uint32_t *) bufinfo2.buf;
            uint32_t val1 = 0;
            uint32_t val2 = 0;
            bool buf1 = true;
            for (pos = 0; pos < maxpos; pos++) {
                if (pwm_fifo_available()) {
                    if (buf1) {
                        val1 = pos < max1 ? *p1 : val1;
                        if (pwm_fifo_queue(val1)) {
                            p1++;
                            buf1 = false;
                        }
                    } else {
                        val2 = pos < max2 ? *p2 : val2;
                        if (pwm_fifo_queue(val2)) {
                            p2++;
                            buf1 = true;
                        }
                    }
                }
                if (elapsed_from(t_start) > timeout) {
                    mp_raise_OSError(MP_EBUSY);
                }
            }
        }
    }
    return mp_obj_new_int(pos*4);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_pwm_fifo_write_obj, 0, machine_pwm_fifo_write);

/// \method fifo_queue(val1[, val2])
/// queue 32-bit word(s) into FIFO
STATIC mp_obj_t machine_pwm_fifo_queue(size_t n_args, const mp_obj_t *args) {
    pwm_fifo_queue(mp_obj_get_int(args[1]));
    if (n_args == 3) {
        pwm_fifo_queue(mp_obj_get_int(args[2]));
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_fifo_queue_obj, 2, 3, machine_pwm_fifo_queue);

/// \method fifo_clear()
/// clear FIFO
STATIC mp_obj_t machine_pwm_fifo_clear(mp_obj_t self_in) {
    pwm_fifo_clear();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pwm_fifo_clear_obj, machine_pwm_fifo_clear);

/// \method id()
/// returns PWM id
STATIC mp_obj_t machine_pwm_id(mp_obj_t self_in) {
    return mp_obj_new_int(((machine_pwm_obj_t *)self_in)->id);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_pwm_id_obj, machine_pwm_id);

STATIC const mp_rom_map_elem_t machine_pwm_locals_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pwm_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_pwm_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_period), MP_ROM_PTR(&machine_pwm_period_obj) },
    { MP_ROM_QSTR(MP_QSTR_duty_ticks), MP_ROM_PTR(&machine_pwm_duty_ticks_obj) },
    { MP_ROM_QSTR(MP_QSTR_duty_u16), MP_ROM_PTR(&machine_pwm_duty_u16_obj) },

    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&machine_pwm_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&machine_pwm_freq_obj) },
    { MP_ROM_QSTR(MP_QSTR_fifo_write), MP_ROM_PTR(&machine_pwm_fifo_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_fifo_queue), MP_ROM_PTR(&machine_pwm_fifo_queue_obj) },
    { MP_ROM_QSTR(MP_QSTR_fifo_clear), MP_ROM_PTR(&machine_pwm_fifo_clear_obj) },
    { MP_ROM_QSTR(MP_QSTR_fifo_active), MP_ROM_PTR(&machine_pwm_fifo_active_obj) },
    { MP_ROM_QSTR(MP_QSTR_id), MP_ROM_PTR(&machine_pwm_id_obj) },
    // class constants
    { MP_ROM_QSTR(MP_QSTR_MS_DISABLE), MP_ROM_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_MS_ENABLE), MP_ROM_INT(1) },
    { MP_ROM_QSTR(MP_QSTR_MODE_PWM), MP_ROM_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_MODE_SERIALIZER), MP_ROM_INT(1) },
};
STATIC MP_DEFINE_CONST_DICT(machine_pwm_locals, machine_pwm_locals_table);

const mp_obj_type_t machine_pwm_type = {
    { &mp_type_type },
    .name = MP_QSTR_PWM,
    .print = machine_pwm_print,
    .make_new = machine_pwm_make_new,
    .locals_dict = (mp_obj_t) &machine_pwm_locals,
};

