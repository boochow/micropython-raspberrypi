#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "modmachine.h"
#include "rpi.h"
#include "bcm283x_gpio.h"
#include "bcm283x_clockmgr.h"
#include "bcm283x_pwm.h"

#define DEFAULT_RANGE 960
#define DEFAULT_DATA  480

typedef struct _machine_pwm_obj_t {
    const mp_obj_base_t base;
    const uint32_t id;
    int32_t pin;
} machine_pwm_obj_t;

static machine_pwm_obj_t machine_pwm_obj[] = {
    {{&machine_pwm_type}, 0, -1},
    {{&machine_pwm_type}, 1, -1},
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

static void pwm_set_ctl(uint32_t id, uint32_t flags) {
    if (id < 2) {
        uint32_t mask = ~(0xff << id * 8);
        pwm_t *pwm = (pwm_t *)(PWM);
        pwm->CTL = (pwm->CTL & mask) | flags << id * 8;
    }
}

static uint32_t pwm_get_ctl(uint32_t id) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        return pwm->CTL & 0xff;
    } else {
        return (pwm->CTL >> 8) & 0xff;
    }
}

static bool pwm_is_active(uint32_t id) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        return ((pwm->CTL & CTL_PWEN1) > 0);
    } else if (id == 1) {
        return ((pwm->CTL & CTL_PWEN2) > 0);
    }
    return false;
}

static void pwm_set_active(uint32_t id, bool active) {
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

static uint32_t pwm_get_range(uint32_t id) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        return pwm->RNG1;
    } else {
        return pwm->RNG2;
    }
}

static uint32_t pwm_get_data(uint32_t id) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        return pwm->DAT1;
    } else {
        return pwm->DAT2;
    }
}

static void pwm_set_range(uint32_t id, uint32_t range) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        pwm->RNG1 = range;
    } else {
        pwm->RNG2 = range;
    }
}

static void pwm_set_data(uint32_t id, uint32_t data) {
    pwm_t *pwm = (pwm_t *)(PWM);
    if (id == 0) {
        pwm->DAT1 = data;
    } else {
        pwm->DAT2 = data;
    }
}

STATIC mp_obj_t machine_pwm_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_id, ARG_pin, ARG_active, ARG_ms, ARG_mode, ARG_polarity, ARG_silence, ARG_use_fifo, ARG_fifo_repeat, ARG_range, ARG_data};
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_id,       MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_pin,      MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_active,   MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_ms,       MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} },
        { MP_QSTR_mode,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_polarity, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_silence,  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_use_fifo, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_fifo_repeat, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_range,    MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_RANGE} },
        { MP_QSTR_data,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = DEFAULT_DATA} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    // parse args
    mp_arg_check_num(n_args, n_kw, 0, MP_OBJ_FUN_ARGS_MAX, true);
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    int id = args[ARG_id].u_int;
    int pin_num = -1;
    // set up GPIO alternate function and get PWM id if the pin is specified
    if (args[ARG_pin].u_obj != mp_const_none) {
        int id_enabled;
        if (mp_obj_is_integer(args[ARG_pin].u_obj)) {
            pin_num = mp_obj_get_int(args[ARG_pin].u_obj);
/*
        } else if (mp_obj_get_type(args[ARG_pin].uobj) == &machine_pin_type) {
            // get pin number from pin object and set it to pin_num
*/
        } else {
            mp_raise_ValueError("invalid pin");
        }
        id_enabled = pwm_select_pin(pin_num);
        if ((id >=0) && (id_enabled != id)) {
            mp_raise_ValueError("invalid PWM number and pin");
        } else {
            id = id_enabled;
        }
    } else if (id > 2) {
        mp_raise_ValueError("invalid PWM number");
    }
    machine_pwm_obj_t *self = (machine_pwm_obj_t*) &machine_pwm_obj[id];
    self->pin = pin_num;
    // initialize PWM controller
    uint32_t flags = 0;
    if (args[ARG_active].u_int) {
        flags |= CTL_PWEN1;
    }
    if (args[ARG_ms].u_int) {
        flags |= CTL_MSEN1;
    }
    if (args[ARG_mode].u_int) {
        flags |= CTL_MODE1;
    }
    if (args[ARG_polarity].u_int) {
        flags |= CTL_POLA1;
    }
    if (args[ARG_silence].u_int) {
        flags |= CTL_SBIT1;
    }
    if (args[ARG_use_fifo].u_int) {
        flags |= CTL_USEF1;
    }
    if (args[ARG_fifo_repeat].u_int) {
        flags |= CTL_RPTL1;
    }
    pwm_set_ctl(id, flags);

    // set range and data
    pwm_set_range(id, args[ARG_range].u_int);
    pwm_set_data(id, args[ARG_data].u_int);

    return self;
}

STATIC void machine_pwm_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pwm_obj_t *self = self_in;
    mp_printf(print, "PWM(%u", self->id);
    if (self->pin >= 0) {
        mp_printf(print, ", pin=%u", self->pin);
    }

    uint32_t flags = pwm_get_ctl(self->id);
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

    mp_printf(print, ", range=%u", pwm_get_range(self->id));
    mp_printf(print, ", data=%u", pwm_get_data(self->id));
    mp_printf(print, ", polarity=%u", (flags & CTL_POLA1) ? 1 : 0);
    mp_printf(print, ", silence=%u", (flags & CTL_SBIT1) ? 1 : 0);
    mp_printf(print, ", use_fifo=%u", (flags & CTL_USEF1) ? 1 : 0);
    mp_printf(print, ", fifo_repeat=%u)", (flags & CTL_RPTL1) ? 1 : 0);
}

/// \method range([value])
/// Get or set the pwm range.
STATIC mp_obj_t machine_pwm_range(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        return MP_OBJ_NEW_SMALL_INT(pwm_get_range(self->id));
    } else {
        // set
        pwm_set_range(self->id, mp_obj_get_int(args[1]));
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_range_obj, 1, 2, machine_pwm_range);

/// \method data([value])
/// Get or set the pwm data.
STATIC mp_obj_t machine_pwm_data(size_t n_args, const mp_obj_t *args) {
    machine_pwm_obj_t *self = args[0];
    if (n_args == 1) {
        // get
        return MP_OBJ_NEW_SMALL_INT(pwm_get_data(self->id));
    } else {
        // set
        pwm_set_data(self->id, mp_obj_get_int(args[1]));
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pwm_data_obj, 1, 2, machine_pwm_data);

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

STATIC const mp_rom_map_elem_t machine_pwm_locals_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_range), MP_ROM_PTR(&machine_pwm_range_obj) },
    { MP_ROM_QSTR(MP_QSTR_data), MP_ROM_PTR(&machine_pwm_data_obj) },
    { MP_ROM_QSTR(MP_QSTR_active), MP_ROM_PTR(&machine_pwm_active_obj) },
/*
    { MP_ROM_QSTR(MP_QSTR_freq), MP_ROM_PTR(&machine_pwm_freq_obj) },
    { MP_ROM_QSTR(MP_QSTR_duty), MP_ROM_PTR(&machine_pwm_duty_obj) },
*/
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

