#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "bcm283x_gpio.h"
#include "bcm283x_i2c.h"
#include "rpi.h"
#include "i2c.h"
#include "modmachine.h"

#define I2C_DEFAULT_FREQ (100000)

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

static void i2c_gpio_setup(uint32_t id, bool on) {
    if (on) {
        switch(id) {
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
    } else {
        switch(id) {
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
    }
}


enum { ARG_freq };
static const mp_arg_t allowed_args[] = {
    { MP_QSTR_freq,       MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = I2C_DEFAULT_FREQ} },
};

STATIC mp_obj_t machine_i2c_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    // parse args
    int id = mp_obj_get_int(all_args[0]);
    if (id > 2) {
        mp_raise_ValueError("invalid bus number");
    }
    machine_i2c_obj_t *i2c = (machine_i2c_obj_t*) &machine_i2c_obj[id];
    mp_arg_parse_all_kw_array(--n_args, n_kw, ++all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set up GPIO alternate function
    i2c_gpio_setup(id, true);

    // initialize I2C controller
    i2c_init(i2c->i2c);
    i2c_set_clock_speed(i2c->i2c, args[0].u_int);

    return i2c;
}

STATIC mp_obj_t machine_i2c_init_helper(machine_i2c_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set up GPIO alternate function
    i2c_gpio_setup(self->id, true);

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
    i2c_gpio_setup(self->id, false);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_i2c_deinit_obj, machine_i2c_deinit);

STATIC void machine_i2c_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_i2c_obj_t *self = self_in;
    uint32_t freq = i2c_get_clock_speed(self->i2c);
    mp_printf(print, "I2C(%u", self->id);
    if (freq != I2C_DEFAULT_FREQ) {
        mp_printf(print, ", freq=%u", freq);
    }
    mp_printf(print, ")");
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
        mp_raise_OSError(-ret);
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

STATIC int read_mem(mp_obj_t self_in, uint16_t addr, uint32_t memaddr, uint8_t addrsize, uint8_t *buf, size_t len) {
    machine_i2c_obj_t *self = (machine_i2c_obj_t*) MP_OBJ_TO_PTR(self_in);

    // write memory address
    i2c_set_slave(self->i2c, addr);

    uint8_t memaddr_buf[4];
    size_t memaddr_len = 0;
    for (int16_t i = addrsize - 8; i >= 0; i -= 8) {
        memaddr_buf[memaddr_len++] = memaddr >> i;
    }
    int ret = i2c_write(self->i2c, memaddr_buf, memaddr_len, false);
    if (ret < 0) {
        i2c_clear_fifo(self->i2c);
    } else {
        // read from the slave
        ret = i2c_read(self->i2c, buf, len);
        if (ret < 0) {
            i2c_clear_fifo(self->i2c);
        }
    }
    return ret;
}

STATIC const mp_arg_t machine_i2c_mem_allowed_args[] = {
    { MP_QSTR_addr,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_memaddr, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_arg,     MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_addrsize, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
};

STATIC mp_obj_t machine_i2c_readfrom_mem(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_addr, ARG_memaddr, ARG_n, ARG_addrsize };
    mp_arg_val_t args[MP_ARRAY_SIZE(machine_i2c_mem_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(machine_i2c_mem_allowed_args), machine_i2c_mem_allowed_args, args);

    // create the buffer to store data into
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(args[ARG_n].u_obj));

    // do the transfer
    int ret = read_mem(pos_args[0],                   \
                       args[ARG_addr].u_int,          \
                       args[ARG_memaddr].u_int,       \
                       args[ARG_addrsize].u_int,      \
                       (uint8_t*)vstr.buf, vstr.len);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }

    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_readfrom_mem_obj, 1, machine_i2c_readfrom_mem);

STATIC mp_obj_t machine_i2c_readfrom_mem_into(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_addr, ARG_memaddr, ARG_buf, ARG_addrsize };
    mp_arg_val_t args[MP_ARRAY_SIZE(machine_i2c_mem_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(machine_i2c_mem_allowed_args), machine_i2c_mem_allowed_args, args);

    // get the buffer to store data into
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_WRITE);

    // do the transfer
    int ret = read_mem(pos_args[0],                   \
                       args[ARG_addr].u_int,          \
                       args[ARG_memaddr].u_int,       \
                       args[ARG_addrsize].u_int,      \
                       bufinfo.buf, bufinfo.len);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_readfrom_mem_into_obj, 1, machine_i2c_readfrom_mem_into);

#define MAX_MEMADDR_SIZE (4)

STATIC mp_obj_t machine_i2c_writeto_mem(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_addr, ARG_memaddr, ARG_buf, ARG_addrsize };
    mp_arg_val_t args[MP_ARRAY_SIZE(machine_i2c_mem_allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args,
        MP_ARRAY_SIZE(machine_i2c_mem_allowed_args), machine_i2c_mem_allowed_args, args);
    machine_i2c_obj_t *self = (machine_i2c_obj_t*) MP_OBJ_TO_PTR(pos_args[0]);

    // get the buffer to write the data from
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buf].u_obj, &bufinfo, MP_BUFFER_READ);

    // create the buffer to send memory address
    uint8_t buf_memaddr[MAX_MEMADDR_SIZE];
    size_t memaddr_len = 0;
    for (int16_t i = args[ARG_addrsize].u_int - 8; i >= 0; i -= 8) {
        buf_memaddr[memaddr_len++] = args[ARG_memaddr].u_int >> i;
    }

    // do the transfer
    i2c_set_slave(self->i2c, args[ARG_addr].u_int);

    int ret = i2c_write(self->i2c, buf_memaddr, memaddr_len, false);
    if (ret < 0) {
        i2c_clear_fifo(self->i2c);
        mp_raise_OSError(-ret);
    }

    ret = i2c_write(self->i2c, bufinfo.buf, bufinfo.len, true);
    if (ret < 0) {
        i2c_clear_fifo(self->i2c);
        mp_raise_OSError(-ret);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_i2c_writeto_mem_obj, 1, machine_i2c_writeto_mem);

STATIC const mp_rom_map_elem_t machine_i2c_locals_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_i2c_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_i2c_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_scan), MP_ROM_PTR(&machine_i2c_scan_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeto), MP_ROM_PTR(&machine_i2c_writeto_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom), MP_ROM_PTR(&machine_i2c_readfrom_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom_into), MP_ROM_PTR(&machine_i2c_readfrom_into_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom_mem), MP_ROM_PTR(&machine_i2c_readfrom_mem_obj) },
    { MP_ROM_QSTR(MP_QSTR_readfrom_mem_into), MP_ROM_PTR(&machine_i2c_readfrom_mem_into_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeto_mem), MP_ROM_PTR(&machine_i2c_writeto_mem_obj) },
};
STATIC MP_DEFINE_CONST_DICT(machine_i2c_locals, machine_i2c_locals_table);

const mp_obj_type_t machine_i2c_type = {
    { &mp_type_type },
    .name = MP_QSTR_I2C,
    .print = machine_i2c_print,
    .make_new = machine_i2c_make_new,
    .locals_dict = (mp_obj_t) &machine_i2c_locals,
};

