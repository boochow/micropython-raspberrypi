#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/obj.h"
#include "py/mperrno.h"
#include "bcm283x_mailbox.h"
#include "vc_property.h"
#include "machine_vc_property.h"

STATIC mp_obj_t machine_vc_property(size_t n_args, const mp_obj_t *args) {
    mp_obj_t result = mp_const_none;

    if (n_args > 0) {
        unsigned int tag = mp_obj_get_int(args[0]);
        uint32_t *buf;
        uint32_t bufsize = vc_property_length(tag);

        if (bufsize == -2) {
            mp_raise_ValueError("unknown tag");
        } else {
            if (bufsize == -1) {
                if ((tag == 0x0004400b) || (tag == 0x0004800b)) {
                    // palette
                    bufsize = 1032;
                } else {
                    // all clocks or command line
                    bufsize = 1024;
                }
            }

            // allocate buffer for mailbox
            buf = (uint32_t *) m_new(uint8_t, bufsize + 24);
            buf[0] = bufsize + 24; // 24 bytes for buf[0..4] and end tag
            buf[1] = MB_PROP_REQUEST;
            buf[2] = tag;
            buf[3] = bufsize;
            buf[4] = 0;

            // store args into mailbox
            int len = 0;
            if ((n_args == 2) && (! mp_obj_is_integer(args[1]))) {
                mp_buffer_info_t bufinfo;
                // second arg may be buffer type or list or tuple
                if (mp_get_buffer(args[1], &bufinfo, MP_BUFFER_READ)) {
                    buf[3] = bufinfo.len;
                    memcpy(&buf[5], bufinfo.buf, bufinfo.len);
                    len = (bufinfo.len + 3) >> 2;
                } else if (mp_obj_get_type(args[1]) == &mp_type_list) {
                    mp_obj_list_t *o = (mp_obj_list_t *) args[1];
                    buf[3] = o->len * 4;
                    for (int i = 0; i < o->len; i++) {
                        buf[5 + i] = mp_obj_get_int(o->items[i]);
                    }
                    len = o->len;
                } else if (mp_obj_get_type(args[1]) == &mp_type_tuple) {
                    mp_obj_tuple_t *o = (mp_obj_tuple_t *) args[1];
                    buf[3] = o->len * 4;
                    for (int i = 0; i < o->len; i++) {
                        buf[5 + i] = mp_obj_get_int(o->items[i]);
                    }
                    len = o->len;
                } else {
                    mp_raise_ValueError("Invalid arguments");
                }
            } else {
                // all arguments should be integers
                for (int i = 1; i < n_args; i++) {
                    if (mp_obj_is_integer(args[i])) {
                        buf[4 + i] = mp_obj_get_int(args[i]);
                    } else {
                        mp_raise_ValueError("Invalid arguments");
                    }
                }
                len = n_args - 1;
            }
            // end tag
            buf[5 + len] = 0;

//            for (int i = 0; i < 5 + n_args; i++) {
//                printf("%08x ", (unsigned int) buf[i]);
//            }
//            printf("\n");

            // request property through mailbox
            mailbox_write(MB_CH_PROP_ARM, (uint32_t) (buf + 0x40000000) >> 4);
            mailbox_read(MB_CH_PROP_ARM);
            
//            for (int i = 0; i < 5 + n_args; i++) {
//                printf("%08x ", (unsigned int) buf[i]);
//            }
//            printf("\n");

            if (buf[1] == MB_PROP_SUCCESS) {
                if (buf[4] & MB_PROP_SUCCESS) {
                    uint32_t datalen = buf[4] & 0x7fffffff;
                    if (datalen == 0) {
                        // no data to return
                        result = mp_const_none;
                    } else if (datalen == 4) {
                        // return an int32 value
                        result = mp_obj_new_int(buf[5]);
                    } else if ((tag == 0x00010003) || (tag == 0x00030020)) {
                        // return bytes (MAC address or EDID block)
                        result = mp_obj_new_bytes((unsigned char *)&buf[5], datalen);
                    } else if (tag == 0x00050001) {
                        // return command line string
                        vstr_t vstr;
                        vstr_init_len(&vstr, datalen);
                        if (vstr.len > 0) {
                            memcpy(vstr.buf, &buf[5], vstr.len);
                            result = mp_obj_new_str_from_vstr(&mp_type_bytes, &vstr);
                        } else {
                            mp_raise_OSError(MP_ENOMEM);
                        }
                    } else {
                        // return a tuple of uint32_t
                        result = mp_obj_new_tuple(datalen >> 2, NULL);
                        mp_obj_tuple_t *o = (mp_obj_tuple_t *) result;
                        for (size_t i = 0; i < o->len; i++) {
                            o->items[i] = mp_obj_new_int(buf[5 + i]);
                        }
                    }
                } else {
                    // failed getting this property value
                    mp_raise_OSError(MP_EIO);
                }
            } else {
                // failed getting property values
                mp_raise_OSError(MP_EIO);
            }
            m_free(buf);
        }
    }
    return result;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(machine_vc_property_obj, 1, 5, machine_vc_property);
