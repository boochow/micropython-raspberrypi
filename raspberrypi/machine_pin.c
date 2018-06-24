#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "bcm283x_gpio.h"
#include "modmachine.h"
#include "extmod/virtpin.h"

typedef struct _machine_pin_obj_t {
    const mp_obj_base_t base;
    const uint32_t id;
    uint32_t pull_mode; // to save pull up/down settings
} machine_pin_obj_t;

STATIC machine_pin_obj_t machine_pin_obj[] = {
    {{&machine_pin_type}, 0, 0},
    {{&machine_pin_type}, 1, 0},
    {{&machine_pin_type}, 2, 0},
    {{&machine_pin_type}, 3, 0},
    {{&machine_pin_type}, 4, 0},
    {{&machine_pin_type}, 5, 0},
    {{&machine_pin_type}, 6, 0},
    {{&machine_pin_type}, 7, 0},
    {{&machine_pin_type}, 8, 0},
    {{&machine_pin_type}, 9, 0},
    {{&machine_pin_type}, 10, 0},
    {{&machine_pin_type}, 11, 0},
    {{&machine_pin_type}, 12, 0},
    {{&machine_pin_type}, 13, 0},
    {{&machine_pin_type}, 14, 0},
    {{&machine_pin_type}, 15, 0},
    {{&machine_pin_type}, 16, 0},
    {{&machine_pin_type}, 17, 0},
    {{&machine_pin_type}, 18, 0},
    {{&machine_pin_type}, 19, 0},
    {{&machine_pin_type}, 20, 0},
    {{&machine_pin_type}, 21, 0},
    {{&machine_pin_type}, 22, 0},
    {{&machine_pin_type}, 23, 0},
    {{&machine_pin_type}, 24, 0},
    {{&machine_pin_type}, 25, 0},
    {{&machine_pin_type}, 26, 0},
    {{&machine_pin_type}, 27, 0},
    {{&machine_pin_type}, 28, 0},
    {{&machine_pin_type}, 29, 0},
    {{&machine_pin_type}, 30, 0},
    {{&machine_pin_type}, 31, 0},
    {{&machine_pin_type}, 32, 0},
    {{&machine_pin_type}, 33, 0},
    {{&machine_pin_type}, 34, 0},
    {{&machine_pin_type}, 35, 0},
    {{&machine_pin_type}, 36, 0},
    {{&machine_pin_type}, 37, 0},
    {{&machine_pin_type}, 38, 0},
    {{&machine_pin_type}, 39, 0},
    {{&machine_pin_type}, 40, 0},
    {{&machine_pin_type}, 41, 0},
    {{&machine_pin_type}, 42, 0},
    {{&machine_pin_type}, 43, 0},
    {{&machine_pin_type}, 44, 0},
    {{&machine_pin_type}, 45, 0},
    {{&machine_pin_type}, 46, 0},
    {{&machine_pin_type}, 47, 0},
    {{&machine_pin_type}, 48, 0},
    {{&machine_pin_type}, 49, 0},
    {{&machine_pin_type}, 50, 0},
    {{&machine_pin_type}, 51, 0},
    {{&machine_pin_type}, 52, 0},
    {{&machine_pin_type}, 53, 0},
};

static void pin_set_pull_mode(uint32_t pin, uint32_t pud) {
    gpio_set_pull_mode(pin, pud);
    machine_pin_obj[pin].pull_mode = pud;
}

// Reading back the pull up/down settings is impossible for BCM2835.
static uint32_t pin_get_pull_mode(uint32_t pin) {
    return machine_pin_obj[pin].pull_mode;
}

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "Pin(%u", self->id);
    uint32_t mode = gpio_get_mode(self->id);
    qstr mode_qst = MP_QSTR_IN;
    if (mode == GPF_OUTPUT) {
        mode_qst = MP_QSTR_OUT;
    }
    mp_printf(print, ", mode=Pin.%s", qstr_str(mode_qst));
    if (mode == GPF_INPUT) {
        uint32_t pull = pin_get_pull_mode(self->id);
        qstr pull_qst = MP_QSTR_PULL_NONE;
        if (pull == GPPUD_EN_UP) {
            pull_qst = MP_QSTR_PULL_UP;
        } else if (pull == GPPUD_EN_DOWN) {
            pull_qst = MP_QSTR_PULL_DOWN;
        }
        mp_printf(print, ", pull=Pin.%s", qstr_str(pull_qst));
    }
    mp_printf(print, ")");
}

// pin.init(mode, pull=None, *, value)
STATIC mp_obj_t machine_pin_obj_init_helper(const machine_pin_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_mode, ARG_pull, ARG_value };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_pull, MP_ARG_OBJ, {.u_obj = mp_const_none}},
        { MP_QSTR_value, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL}},
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set initial value (do this before configuring mode/pull)
    if (args[ARG_value].u_obj != MP_OBJ_NULL) {
        gpio_set_level(self->id, mp_obj_is_true(args[ARG_value].u_obj));
    }

    // configure mode
    if (args[ARG_mode].u_obj != mp_const_none) {
        mp_int_t pin_io_mode = mp_obj_get_int(args[ARG_mode].u_obj);
        if (self->id > 53) {
            mp_raise_ValueError("pin not found");
        } else {
            gpio_set_mode(self->id, pin_io_mode);
        }
    }

    // configure pull
    if (args[ARG_pull].u_obj != mp_const_none) {
        pin_set_pull_mode(self->id, mp_obj_get_int(args[ARG_pull].u_obj));
    }

    return mp_const_none;
}

// constructor(id, ...)
mp_obj_t mp_pin_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // get the wanted pin object
    int wanted_pin = mp_obj_get_int(args[0]);
    const machine_pin_obj_t *self = NULL;
    if (0 <= wanted_pin && wanted_pin < MP_ARRAY_SIZE(machine_pin_obj)) {
        self = (machine_pin_obj_t*)&machine_pin_obj[wanted_pin];
    } else {
        mp_raise_ValueError("invalid pin");
    }

    if (n_args > 1 || n_kw > 0) {
        // pin mode given, so configure this GPIO
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        machine_pin_obj_init_helper(self, n_args - 1, args + 1, &kw_args);
    }

    return MP_OBJ_FROM_PTR(self);
}

// fast method for getting/setting pin value
STATIC mp_obj_t machine_pin_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);
    machine_pin_obj_t *self = self_in;
    if (n_args == 0) {
        // get pin
        return MP_OBJ_NEW_SMALL_INT(gpio_get_level(self->id));
    } else {
        // set pin
        gpio_set_level(self->id, mp_obj_is_true(args[0]));
        return mp_const_none;
    }
}

// pin.init(mode, pull)
STATIC mp_obj_t machine_pin_obj_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_pin_obj_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_pin_init_obj, 1, machine_pin_obj_init);

// pin.value([value])
STATIC mp_obj_t machine_pin_value(size_t n_args, const mp_obj_t *args) {
    return machine_pin_call(args[0], n_args - 1, 0, args + 1);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_pin_value_obj, 1, 2, machine_pin_value);


STATIC mp_uint_t pin_ioctl(mp_obj_t self_in, mp_uint_t request, uintptr_t arg, int *errcode) {
    (void)errcode;
    machine_pin_obj_t *self = self_in;

    switch (request) {
        case MP_PIN_READ: {
            return gpio_get_level(self->id);
        }
        case MP_PIN_WRITE: {
            gpio_set_level(self->id, arg);
            return 0;
        }
    }
    return -1;
}

STATIC const mp_pin_p_t pin_pin_p = {
  .ioctl = pin_ioctl,
};

STATIC const mp_rom_map_elem_t machine_pin_locals_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_pin_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_value), MP_ROM_PTR(&machine_pin_value_obj) },

    // class constants
    { MP_ROM_QSTR(MP_QSTR_IN), MP_ROM_INT(GPF_INPUT) },
    { MP_ROM_QSTR(MP_QSTR_OUT), MP_ROM_INT(GPF_OUTPUT) },
    { MP_ROM_QSTR(MP_QSTR_PULL_NONE), MP_ROM_INT(GPPUD_NONE) },
    { MP_ROM_QSTR(MP_QSTR_PULL_UP), MP_ROM_INT(GPPUD_EN_UP) },
    { MP_ROM_QSTR(MP_QSTR_PULL_DOWN), MP_ROM_INT(GPPUD_EN_DOWN) },
};
STATIC MP_DEFINE_CONST_DICT(machine_pin_locals, machine_pin_locals_table);

const mp_obj_type_t machine_pin_type = {
    { &mp_type_type },
    .name = MP_QSTR_Pin,
    .print = machine_pin_print,
    .make_new = mp_pin_make_new,
    .call = machine_pin_call,
    .protocol = &pin_pin_p,
    .locals_dict = (mp_obj_t)&machine_pin_locals,
};

