#include <stdio.h>
#include <stdint.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/obj.h"

#include "gpu.h"
#include "gpu_vc_property.h"

static fb_info_t fb_info = {1920, 1080, 480, 270, 0, 16, 0, 0, 0, 0};

/// \method fb_init(w=640, h=480, bpp=16, screen_w=0, screen_h=0, offset_x=0, offset_y=0)
/// This initialises frame buffer resolution, bpp, video signal settings.
/// When called, GPU allocates framebuffer memory.
/// Parameters:
///  - w, h, bpp : actual frame buffer size and bpp
///  - screen_w, screen_h : display resolution ; 0 means value is same as w,h
///  - offset_x, offset_y : framebuffer position from screen's top left corner
/// Static variable fb_info holds framebuffer settings.
///  - w, h, bpp, screen_w, screen_h, offset_x, offset_y: passed as parameter
///  - rowbytes : number of bytes in a row
///  - buf_addr : pointer to the frame buffer allocated by GPU
///  - buf_size : number of bytes of the frame buffer
STATIC mp_obj_t gpu_fb_init(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_w,        MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 640} },
        { MP_QSTR_h,        MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 480} },
        { MP_QSTR_bpp,      MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 16} },
        { MP_QSTR_screen_w, MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_screen_h, MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_offset_x, MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 0} },
        { MP_QSTR_offset_y, MP_ARG_KW_ONLY  | MP_ARG_INT, {.u_int = 0} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // set the GPU configuration values
    fb_info.w = args[0].u_int;
    fb_info.h = args[1].u_int;
    fb_info.bpp = args[2].u_int;
    fb_info.screen_w = args[3].u_int != 0 ? args[3].u_int : args[0].u_int;
    fb_info.screen_h = args[4].u_int != 0 ? args[4].u_int : args[1].u_int;
    fb_info.offset_x = args[5].u_int;
    fb_info.offset_y = args[6].u_int;

    // init the video core
    rpi_fb_init(&fb_info);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(gpu_fb_init_obj, 2, gpu_fb_init);

/// \function gpu.fb_data()
/// This returns the pointer to frame buffer as byte array if it is allocated.
STATIC mp_obj_t gpu_fb_data(void) {
    if (fb_info.buf_addr == 0) {
        return mp_const_none;
    } else {
        return mp_obj_new_bytearray_by_ref(fb_info.buf_size, (void *) fb_info.buf_addr);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(gpu_fb_data_obj, gpu_fb_data);

/// \function gpu.fb_rowbytes()
/// This returns the number of bytes in a row of the frame buffer.
STATIC mp_obj_t gpu_fb_rowbytes(void) {
    if (fb_info.rowbytes == 0) {
        return mp_const_none;
    } else {
        return mp_obj_new_int(fb_info.rowbytes);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(gpu_fb_rowbytes_obj, gpu_fb_rowbytes);



STATIC const mp_rom_map_elem_t gpu_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_gpu) },

    { MP_ROM_QSTR(MP_QSTR_fb_init), MP_ROM_PTR(&gpu_fb_init_obj) },
    { MP_ROM_QSTR(MP_QSTR_fb_data), MP_ROM_PTR(&gpu_fb_data_obj) },
    { MP_ROM_QSTR(MP_QSTR_fb_rowbytes), MP_ROM_PTR(&gpu_fb_rowbytes_obj) },
    { MP_ROM_QSTR(MP_QSTR_vc_property), MP_ROM_PTR(&gpu_vc_property_obj) },
};

STATIC MP_DEFINE_CONST_DICT(gpu_module_globals, gpu_module_globals_table);

const mp_obj_module_t gpu_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*) &gpu_module_globals,
};
