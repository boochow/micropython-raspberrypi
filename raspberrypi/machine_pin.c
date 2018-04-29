#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "bcm283x_gpio.h"
#include "modmachine.h"
#include "extmod/virtpin.h"

typedef struct _machine_pin_obj_t {
    mp_obj_base_t base;
    uint32_t id;
} machine_pin_obj_t;

STATIC const machine_pin_obj_t machine_pin_obj[] = {
    {{&machine_pin_type}, 0},
    {{&machine_pin_type}, 1},
    {{&machine_pin_type}, 2},
    {{&machine_pin_type}, 3},
    {{&machine_pin_type}, 4},
    {{&machine_pin_type}, 5},
    {{&machine_pin_type}, 6},
    {{&machine_pin_type}, 7},
    {{&machine_pin_type}, 8},
    {{&machine_pin_type}, 9},
    {{&machine_pin_type}, 10},
    {{&machine_pin_type}, 11},
    {{&machine_pin_type}, 12},
    {{&machine_pin_type}, 13},
    {{&machine_pin_type}, 14},
    {{&machine_pin_type}, 15},
    {{&machine_pin_type}, 16},
    {{&machine_pin_type}, 17},
    {{&machine_pin_type}, 18},
    {{&machine_pin_type}, 19},
    {{&machine_pin_type}, 20},
    {{&machine_pin_type}, 21},
    {{&machine_pin_type}, 22},
    {{&machine_pin_type}, 23},
    {{&machine_pin_type}, 24},
    {{&machine_pin_type}, 25},
    {{&machine_pin_type}, 26},
    {{&machine_pin_type}, 27},
    {{&machine_pin_type}, 28},
    {{&machine_pin_type}, 29},
    {{&machine_pin_type}, 30},
    {{&machine_pin_type}, 31},
    {{&machine_pin_type}, 32},
    {{&machine_pin_type}, 33},
    {{&machine_pin_type}, 34},
    {{&machine_pin_type}, 35},
    {{&machine_pin_type}, 36},
    {{&machine_pin_type}, 37},
    {{&machine_pin_type}, 38},
    {{&machine_pin_type}, 39},
    {{&machine_pin_type}, 40},
    {{&machine_pin_type}, 41},
    {{&machine_pin_type}, 42},
    {{&machine_pin_type}, 43},
    {{&machine_pin_type}, 44},
    {{&machine_pin_type}, 45},
    {{&machine_pin_type}, 46},
    {{&machine_pin_type}, 47},
    {{&machine_pin_type}, 48},
    {{&machine_pin_type}, 49},
    {{&machine_pin_type}, 50},
    {{&machine_pin_type}, 51},
    {{&machine_pin_type}, 52},
    {{&machine_pin_type}, 53},
};

static void gpio_set_mode(uint32_t pin, uint32_t mode) {
    uint32_t reg;
    uint32_t pos;

    reg = GPFSEL0 + pin / 10 * 4;
    pos = (pin % 10) * 3;
    mode = mode & 7U;
    IOREG(reg) = (IOREG(reg) & ~(7 << pos)) | (mode << pos);
}

static uint32_t gpio_get_mode(uint32_t pin) {
    uint32_t reg;
    uint32_t pos;

    reg = GPFSEL0 + pin / 10 * 4;
    pos = (pin % 10) * 3;
    return (IOREG(reg) >> pos) & 7U;
}

static void gpio_set_level(uint32_t pin, uint32_t level) {
    uint32_t reg;

    reg = (pin > 31) ? 4 : 0;
    reg += (level == 0) ? GPCLR0 : GPSET0;
    IOREG(reg) = 1 << (pin & 0x1F);
}

static uint32_t gpio_get_level(uint32_t pin) {
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

static void gpio_set_pull_mode(uint32_t pin, uint32_t pud) {
    uint32_t reg;

    IOREG(GPPUD) = pud;
    delay_cycles(150);
    reg = GPPUDCLK0 + (pin >> 5) * 4;
    IOREG(reg) = 1 << (pin & 0x1f);
    delay_cycles(150);
    IOREG(GPPUD) = 0;
    IOREG(reg) = 0;
}

STATIC void machine_pin_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_pin_obj_t *self = self_in;
    mp_printf(print, "Pin(%u)", self->id);
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
        gpio_set_pull_mode(self->id, mp_obj_get_int(args[ARG_pull].u_obj));
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

