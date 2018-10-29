#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "modmachine.h"
#include "bcm283x_gpio.h"
#include "bcm283x_clockmgr.h"

typedef struct _machine_clock_obj_t {
    mp_obj_base_t base;
    clockmgr_t *clock_reg;
} machine_clock_obj_t;

enum { ARG_enable, ARG_source, ARG_mash, ARG_divi, ARG_divf };
static const mp_arg_t allowed_args[] = {
    { MP_QSTR_enable, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    { MP_QSTR_source, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    { MP_QSTR_mash,   MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    { MP_QSTR_divi,   MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    { MP_QSTR_divf,   MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
};

STATIC mp_obj_t machine_clock_init_helper(machine_clock_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // initialize clock manager
    uint32_t ctl_save = self->clock_reg->CTL;
    uint32_t reg = 0;
    if (mp_obj_is_integer(args[ARG_enable].u_obj)) {
        if (mp_obj_get_int(args[ARG_enable].u_obj) != 0) {
            reg |= CM_CTL_ENAB;
        }
    } else {
        reg |= (ctl_save & CM_CTL_ENAB);
    }

    if (MP_OBJ_IS_INT(args[ARG_source].u_obj)) {
        reg |= mp_obj_get_int(args[ARG_source].u_obj) & CM_CTL_SRC_MASK;
    } else {
        reg |= (ctl_save & CM_CTL_SRC_MASK);
    }

    if (MP_OBJ_IS_INT(args[ARG_mash].u_obj)) {
        reg |= (mp_obj_get_int(args[ARG_mash].u_obj) << 9) & CM_CTL_MASH_MASK;
    } else {
        reg |= (ctl_save & CM_CTL_MASH_MASK);
    }

    if (reg != (ctl_save & (CM_CTL_MASH_MASK | CM_CTL_SRC_MASK))) {
        clockmgr_config_ctl(self->clock_reg, reg);
    }
    // configure clock divisor
    uint32_t divi = 0;
    uint32_t divf = 0;
    if (mp_obj_is_integer(args[ARG_divi].u_obj)) {
        divi = mp_obj_get_int(args[ARG_divi].u_obj);
    }

    if (mp_obj_is_integer(args[ARG_divf].u_obj)) {
        divf = mp_obj_get_int(args[ARG_divi].u_obj);
    }

    if ((divi != 0) || (divf != 0)){
        clockmgr_set_div(self->clock_reg, divi, divf); 
    }

    return (mp_obj_t *) self;
}

STATIC mp_obj_t machine_clock_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_clock_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_clock_init_obj, 1, machine_clock_init);

STATIC mp_obj_t machine_clock_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    // make new clock object
    uint32_t reg = mp_obj_get_int(all_args[0]);
    if ((reg != CM_GP0) && (reg != CM_GP1) && (reg != CM_GP2) && (reg != CM_PCM) && (reg != CM_PWM)){
        mp_raise_ValueError("invalid register address");
    }
    machine_clock_obj_t *self = m_new_obj(machine_clock_obj_t);
    self->base.type = &machine_clock_type;
    self->clock_reg = (clockmgr_t *) reg;
    // initialize clock manager
    if (n_args > 1 || n_kw > 0) {
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, all_args + n_args);
        machine_clock_init_helper(self, n_args - 1, all_args + 1, &kw_args);
    }

    return (mp_obj_t *) self;
}

STATIC mp_obj_t machine_clock_divisor(size_t n_args, const mp_obj_t *args) {
    machine_clock_obj_t *self = (machine_clock_obj_t*) MP_OBJ_TO_PTR(args[0]);
    uint32_t divi = 0;
    uint32_t divf = 0;

    if (n_args == 1) {
        // return a tuple of (divi, divf)
        divi = (self->clock_reg->DIV >> 12) & 0xfffU;
        divf = self->clock_reg->DIV & 0xfffU;
        mp_obj_tuple_t *result = mp_obj_new_tuple(2, NULL);
        result->items[0] = mp_obj_new_int(divi);
        result->items[1] = mp_obj_new_int(divf);
        return (mp_obj_t *) result;
    } else {
        if (mp_obj_is_integer(args[1])) {
            divi = mp_obj_get_int(args[1]);
        }
        if ((n_args == 3) && (mp_obj_is_integer(args[2]))) {
            divf = mp_obj_get_int(args[2]);
        }
        clockmgr_set_div(self->clock_reg, divi, divf); 
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_clock_divisor_obj, 1, 3, machine_clock_divisor);

STATIC const mp_rom_map_elem_t machine_clock_locals_dict_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_clock_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_divisor), MP_ROM_PTR(&machine_clock_divisor_obj) },
    // class constants
    { MP_ROM_QSTR(MP_QSTR_GP0), MP_ROM_INT(CM_GP0) },
    { MP_ROM_QSTR(MP_QSTR_GP1), MP_ROM_INT(CM_GP1) },
    { MP_ROM_QSTR(MP_QSTR_GP2), MP_ROM_INT(CM_GP2) },
    { MP_ROM_QSTR(MP_QSTR_PCM), MP_ROM_INT(CM_PCM) },
    { MP_ROM_QSTR(MP_QSTR_PWM), MP_ROM_INT(CM_PWM) },
    { MP_ROM_QSTR(MP_QSTR_OSC), MP_ROM_INT(CM_CTL_SRC_OSC) },
    { MP_ROM_QSTR(MP_QSTR_PLLC), MP_ROM_INT(CM_CTL_SRC_PLLC) },
    { MP_ROM_QSTR(MP_QSTR_PLLD), MP_ROM_INT(CM_CTL_SRC_PLLD) },
    { MP_ROM_QSTR(MP_QSTR_HDMI), MP_ROM_INT(CM_CTL_SRC_HDMI) },
};

STATIC MP_DEFINE_CONST_DICT(machine_clock_locals_dict, machine_clock_locals_dict_table);

const mp_obj_type_t machine_clock_type = {
    { &mp_type_type },
    .name = MP_QSTR_Clock,
    .make_new = machine_clock_make_new,
    .locals_dict = (mp_obj_dict_t*) &machine_clock_locals_dict,
};

