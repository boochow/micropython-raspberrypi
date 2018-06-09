#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "bcm283x_gpio.h"
#include "bcm283x_i2c.h"
#include "rpi.h"
#include "i2c.h"
#include "modmachine.h"

typedef struct _machine_i2c_obj_t {
    mp_obj_base_t base;
    uint32_t id;
    i2c_t *i2c;
} machine_i2c_obj_t;

const machine_i2c_obj_t machine_i2c_obj[] = {
    {{&machine_i2c_type}, 0, (i2c_t *) BSC0},
    {{&machine_i2c_type}, 1, (i2c_t *) BSC1},
    {{&machine_i2c_type}, 2, (i2c_t *) BSC2},
};

STATIC mp_obj_t machine_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    int id = mp_obj_get_int(args[0]);
    if (id > 2) {
        mp_raise_ValueError("invalid bus number");
    }
    machine_i2c_obj_t *i2c = (machine_i2c_obj_t*) &machine_i2c_obj[id];
    return i2c;
}

STATIC mp_obj_t machine_i2c_init_helper(machine_i2c_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_freq,       MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 100000} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set up GPIO alternate function
    switch(self->id) {
    case 0:
        gpio_set_mode(0, GPF_ALT_0);
        gpio_set_mode(1, GPF_ALT_0);
        break;
    case 1:
        gpio_set_mode(2, GPF_ALT_0);
        gpio_set_mode(3, GPF_ALT_0);
        break;
    case 2:
        break;
    default:
        ;
    }

    // initialize I2C controller
    i2c_init(self->i2c);
    i2c_set_clock_speed(self->i2c, args[0].u_int);

    return mp_const_none;
}

STATIC mp_obj_t machine_i2c_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_i2c_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_init_obj, 0, machine_i2c_init);

STATIC mp_obj_t machine_i2c_deinit(mp_obj_t self_in) {
    machine_i2c_obj_t *self = self_in;

    // de-initialize I2C controller
    i2c_deinit(self->i2c);

    // set GPIO to input mode
    switch(self->id) {
    case 0:
        gpio_set_mode(0, GPF_INPUT);
        gpio_set_mode(1, GPF_INPUT);
        break;
    case 1:
        gpio_set_mode(2, GPF_INPUT);
        gpio_set_mode(3, GPF_INPUT);
        break;
    case 2:
        break;
    default:
        ;
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_i2c_deinit_obj, machine_i2c_deinit);

STATIC void machine_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_i2c_obj_t *self = self_in;
    mp_printf(print, "I2C(%u)", self->id);
}

/// \method writeto(addr, buf)
/// \method readfrom(addr, num)

STATIC mp_obj_t machine_i2c_writeto(size_t n_args, const mp_obj_t *args) {
    machine_i2c_obj_t *self = (machine_i2c_obj_t*)MP_OBJ_TO_PTR(args[0]);
    mp_int_t addr = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_READ);
    bool stop = (n_args == 3) ? true : mp_obj_is_true(args[3]);

    i2c_set_slave(self->i2c, addr);
    int ret = i2c_write(self->i2c, bufinfo.buf, bufinfo.len, stop);
    if (ret < 0) {
        i2c_clear_fifo(self->i2c);
        if (ret == -1) {
            mp_raise_OSError(MP_EIO);
        } else if (ret == -2) {
            mp_raise_OSError(MP_ETIMEDOUT);
        }
    }

    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_i2c_writeto_obj, 3, 4, machine_i2c_writeto);

STATIC mp_obj_t machine_i2c_readfrom_into(mp_obj_t self_in, mp_obj_t slave, mp_obj_t buf) {
    machine_i2c_obj_t *self = (machine_i2c_obj_t*) MP_OBJ_TO_PTR(self_in);
    mp_int_t addr = mp_obj_get_int(slave);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    i2c_set_slave(self->i2c, addr);
    int ret = i2c_read(self->i2c, (uint8_t*)bufinfo.buf, bufinfo.len);
    if (ret < 0) {
        i2c_clear_fifo(self->i2c);
        if (ret == -1) {
            mp_raise_OSError(MP_EIO);
        } else if (ret == -2) {
            mp_raise_OSError(MP_ETIMEDOUT);
        }
        mp_raise_OSError(-ret);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_i2c_readfrom_into_obj, machine_i2c_readfrom_into);

STATIC mp_obj_t machine_i2c_readfrom(mp_obj_t self_in, mp_obj_t slave, mp_obj_t len) {
    machine_i2c_obj_t *self = (machine_i2c_obj_t*) MP_OBJ_TO_PTR(self_in);
    mp_int_t addr = mp_obj_get_int(slave);
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(len));
    i2c_set_slave(self->i2c, addr);
    int ret = i2c_read(self->i2c, (uint8_t*)vstr.buf, vstr.len);
    if (ret < 0) {
        i2c_clear_fifo(self->i2c);
        if (ret == -1) {
            mp_raise_OSError(MP_EIO);
        } else if (ret == -2) {
            mp_raise_OSError(MP_ETIMEDOUT);
        }
        mp_raise_OSError(-ret);
    }
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_i2c_readfrom_obj, machine_i2c_readfrom);

STATIC mp_obj_t machine_i2c_scan(mp_obj_t self_in) {
    machine_i2c_obj_t *self = (machine_i2c_obj_t*) MP_OBJ_TO_PTR(self_in);
    mp_obj_t list = mp_obj_new_list(0, NULL);
    // 7-bit addresses 0b0000xxx and 0b1111xxx are reserved
    for (int addr = 0x08; addr < 0x78; ++addr) {
        while(i2c_busy(self->i2c));
        mp_hal_delay_ms(1);
        i2c_set_slave(self->i2c, addr);
        uint8_t buf = 0;
        int ret;
        if (((0x30 <= addr) && (addr <= 0x37))
            || ((0x50 <= addr) && (addr <= 0x5f))) {
            ret = i2c_read(self->i2c, &buf, 1);
        } else {
            ret = i2c_write(self->i2c, &buf, 0, true);
        }
        if (ret >= 0) {
            mp_obj_list_append(list, MP_OBJ_NEW_SMALL_INT(addr));
        }
    }
    return list;
}
MP_DEFINE_CONST_FUN_OBJ_1(machine_i2c_scan_obj, machine_i2c_scan);


STATIC const mp_rom_map_elem_t machine_i2c_locals_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_i2c_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_i2c_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&machine_i2c_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeto), MP_ROM_PTR(&machine_i2c_writeto_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom), MP_ROM_PTR(&machine_i2c_readfrom_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom_into), MP_ROM_PTR(&machine_i2c_readfrom_into_obj) },
    // class constants
};
STATIC MP_DEFINE_CONST_DICT(machine_i2c_locals, machine_i2c_locals_table);

const mp_obj_type_t machine_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2c,
    .print = machine_i2c_print,
    .make_new = machine_i2c_make_new,
    .locals_dict = (mp_obj_t) &machine_i2c_locals,
};

