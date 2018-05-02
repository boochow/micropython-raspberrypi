#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "machine_usb_mode.h"
#include "usbhost.h"


// this will be persistent across a soft-reset
mp_uint_t rpi_usb_flags = 0;

STATIC mp_obj_t machine_usb_mode(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_mode, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    // fetch the current usb mode -> machine.usb_mode()
    if (n_args == 0) {
        return MP_OBJ_NEW_QSTR(MP_QSTR_host);
    }

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // record the fact that the usb has been explicitly configured
    rpi_usb_flags |= RPI_USB_FLAG_USB_MODE_CALLED;

    // check if user wants to disable the USB
    if (args[0].u_obj == mp_const_none) {
        // disable usb
        rpi_usb_host_deinit();
        return mp_const_none;
    }

    // get mode string
    const char *mode_str = mp_obj_str_get_str(args[0].u_obj);

    // hardware configured for USB host mode

    if (strcmp(mode_str, "host") == 0) {
        rpi_usb_host_init();
    } else {
        goto bad_mode;
    }

    return mp_const_none;

bad_mode:
    mp_raise_ValueError("bad USB mode");
}
MP_DEFINE_CONST_FUN_OBJ_KW(machine_usb_mode_obj, 0, machine_usb_mode);
