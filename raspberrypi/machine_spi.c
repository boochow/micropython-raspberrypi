#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "py/mperrno.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "bcm283x_gpio.h"
#include "bcm283x_spi.h"
#include "rpi.h"
#include "spi.h"
#include "modmachine.h"

#define MICROPY_PY_MACHINE_SPI_MSB (0)
#define MICROPY_PY_MACHINE_SPI_LSB (1)

typedef struct _machine_spi_obj_t {
    mp_obj_base_t base;
    uint32_t id;
    spi_t *spi;
} machine_spi_obj_t;

const machine_spi_obj_t machine_spi_obj[] = {
    {{&machine_spi_type}, 0, (spi_t *) SPI0},
};

static void spi_gpio_setup(uint32_t id, bool on) {
    if (on) {
        switch(id) {
        case 0:
            gpio_set_mode(9, GPF_ALT_0);  // SPI0_MISO
            gpio_set_mode(10, GPF_ALT_0); // SPI0_MOSI
            gpio_set_mode(11, GPF_ALT_0); // SPI0_SCLK
            break;
        default:
            ;
        }
    } else {
        switch(id) {
        case 0:
            gpio_set_mode(9, GPF_INPUT);
            gpio_set_mode(10, GPF_INPUT);
            gpio_set_mode(11, GPF_INPUT);
        break;
        default:
            ;
        }
    }
}

STATIC mp_obj_t machine_spi_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    int id = mp_obj_get_int(args[0]);
    if (id != 0) {
        mp_raise_ValueError("invalid bus number");
    }
    machine_spi_obj_t *spi = (machine_spi_obj_t*) &machine_spi_obj[id];

    // set up GPIO alternate function
    spi_gpio_setup(id, true);

    // initialize SPI controller
    spi_init(spi->spi, 0, 0);

    return spi;
}

STATIC mp_obj_t machine_spi_init_helper(machine_spi_obj_t *self, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_baudrate, ARG_polarity, ARG_phase, ARG_bits, ARG_firstbit };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_baudrate,  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1000000} },
        { MP_QSTR_polarity,  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_phase,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_bits,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 8} },
        { MP_QSTR_firstbit,  MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = MICROPY_PY_MACHINE_SPI_MSB} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set up GPIO alternate function
    spi_gpio_setup(self->id, true);

    // initialize SPI controller
    if (args[ARG_bits].u_int != 8) {
        mp_raise_ValueError("bits must be 8");
    }
    if (args[ARG_firstbit].u_int != MICROPY_PY_MACHINE_SPI_MSB) {
        mp_raise_ValueError("firstbit must be MSB");
    }
    spi_init(self->spi, args[ARG_polarity].u_int, args[ARG_phase].u_int);
    spi_set_clock_speed(self->spi, args[ARG_baudrate].u_int);

    return mp_const_none;
}

STATIC mp_obj_t machine_spi_init(size_t n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    return machine_spi_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(machine_spi_init_obj, 0, machine_spi_init);

STATIC mp_obj_t machine_spi_deinit(mp_obj_t self_in) {
    machine_spi_obj_t *self = self_in;

    // de-initialize SPI controller
    spi_deinit(self->spi);

    // set GPIO to input mode
    spi_gpio_setup(self->id, false);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_spi_deinit_obj, machine_spi_deinit);

STATIC void machine_spi_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    machine_spi_obj_t *self = self_in;
    mp_printf(print, "SPI(%u)", self->id);
}

STATIC mp_obj_t machine_spi_write(mp_obj_t self_in, mp_obj_t buf) {
    machine_spi_obj_t *self = (machine_spi_obj_t*) MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    int ret = spi_transfer(self->spi, bufinfo.buf, bufinfo.len, NULL, 0);
    if (ret < 0) {
        mp_raise_OSError(-ret);
    }

    return MP_OBJ_NEW_SMALL_INT(ret);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(machine_spi_write_obj, machine_spi_write);

STATIC mp_obj_t machine_spi_write_readinto(mp_obj_t self_in, mp_obj_t wbuf, mp_obj_t rbuf) {
    machine_spi_obj_t *self = (machine_spi_obj_t*) MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t src;
    mp_get_buffer_raise(wbuf, &src, MP_BUFFER_READ);
    mp_buffer_info_t dest;
    mp_get_buffer_raise(rbuf, &dest, MP_BUFFER_WRITE);
    if (src.len != dest.len) {
        mp_raise_ValueError("buffers must be the same length");
    }
    if (src.len > 0) {
        int ret = spi_transfer(self->spi, src.buf, src.len, dest.buf, dest.len);
        if (ret < 0) {
            mp_raise_OSError(-ret);
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_spi_write_readinto_obj, machine_spi_write_readinto);

STATIC mp_obj_t machine_spi_readinto(size_t n_args, const mp_obj_t *args) {
    machine_spi_obj_t *self = (machine_spi_obj_t*) MP_OBJ_TO_PTR(args[0]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[1], &bufinfo, MP_BUFFER_READ);
    if (bufinfo.len > 0) {
        uint8_t wchar = (n_args == 2) ? 0 : mp_obj_get_int(args[2]);
        int ret = spi_transfer(self->spi, &wchar, 1, (uint8_t*)bufinfo.buf, bufinfo.len);
        if (ret < 0) {
            mp_raise_OSError(-ret);
        }
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_readinto_obj, 1, 2, machine_spi_readinto);

STATIC mp_obj_t machine_spi_read(size_t n_args, const mp_obj_t *args) {
    machine_spi_obj_t *self = (machine_spi_obj_t*) MP_OBJ_TO_PTR(args[0]);
    vstr_t vstr;
    vstr_init_len(&vstr, mp_obj_get_int(args[1]));
    if (vstr.len > 0) {
        uint8_t wchar = (n_args == 2) ? 0 : mp_obj_get_int(args[2]);
        int ret = spi_transfer(self->spi, &wchar, 1, (uint8_t*)vstr.buf, vstr.len);
        if (ret < 0) {
            mp_raise_OSError(-ret);
        }
    }
    return mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_spi_read_obj, 1, 2, machine_spi_read);

STATIC const mp_rom_map_elem_t machine_spi_locals_table[] = {
    // instance methods
    { MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&machine_spi_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&machine_spi_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR_write), MP_ROM_PTR(&machine_spi_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_write_readinto), MP_ROM_PTR(&machine_spi_write_readinto_obj) },
    { MP_ROM_QSTR(MP_QSTR_read), MP_ROM_PTR(&machine_spi_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_readinto), MP_ROM_PTR(&machine_spi_readinto_obj) },
    // class constants
    { MP_ROM_QSTR(MP_QSTR_MSB), MP_ROM_INT(MICROPY_PY_MACHINE_SPI_MSB) },
    { MP_ROM_QSTR(MP_QSTR_LSB), MP_ROM_INT(MICROPY_PY_MACHINE_SPI_LSB) },
};
STATIC MP_DEFINE_CONST_DICT(machine_spi_locals, machine_spi_locals_table);

const mp_obj_type_t machine_spi_type = {
    { &mp_type_type },
    .name = MP_QSTR_SPI,
    .print = machine_spi_print,
    .make_new = machine_spi_make_new,
    .locals_dict = (mp_obj_t) &machine_spi_locals,
};

