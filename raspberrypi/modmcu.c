#include <stdio.h>
#include <stdint.h>

#include "py/obj.h"
#include "py/objint.h"
#include "extmod/machine_mem.h"
#include "bcm283x.h"

STATIC const mp_rom_map_elem_t mcu_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_mcu) },

    { MP_ROM_QSTR(MP_QSTR_mem8), MP_ROM_PTR(&machine_mem8_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem16), MP_ROM_PTR(&machine_mem16_obj) },
    { MP_ROM_QSTR(MP_QSTR_mem32), MP_ROM_PTR(&machine_mem32_obj) },
    { MP_ROM_QSTR(MP_QSTR_IO_BASE), MP_ROM_INT(IO_BASE) },
};

STATIC MP_DEFINE_CONST_DICT(mcu_module_globals, mcu_module_globals_table);

const mp_obj_module_t mcu_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*) &mcu_module_globals,
};
