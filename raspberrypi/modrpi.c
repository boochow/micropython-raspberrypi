#include <stdio.h>
#include <stdint.h>

#include "py/obj.h"
#include "py/objint.h"
#include "extmod/machine_mem.h"
#include "gpio_registers.h"

STATIC const mp_rom_map_elem_t rpi_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_rpi) },

    { MP_ROM_QSTR(MP_QSTR_mem8), MP_ROM_PTR(&machine_mem8_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem16), MP_ROM_PTR(&machine_mem16_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem32), MP_ROM_PTR(&machine_mem32_obj) },

};

STATIC MP_DEFINE_CONST_DICT(rpi_module_globals, rpi_module_globals_table);

const mp_obj_module_t rpi_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&rpi_module_globals,
};
