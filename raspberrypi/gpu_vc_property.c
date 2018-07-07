#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/obj.h"
#include "py/mperrno.h"
#include "bcm283x_mailbox.h"
#include "vc_property.h"
#include "gpu_vc_property.h"

STATIC mp_obj_t gpu_vc_property(size_t n_args, const mp_obj_t *args) {
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
            // 24 bytes for buf[0..4] and end tag, + padding, 16 bytes aligned
            unsigned int msgsize = (bufsize + 24 + 15) & 0xfffffff0;
//            unsigned int msgsize = bufsize + 24;
            buf = (uint32_t *) m_new(uint8_t, msgsize);
            buf[0] = msgsize; 
            buf[1] = MB_PROP_REQUEST;
            buf[2] = tag;
            buf[3] = bufsize;
            buf[4] = 0;

            // store args into mailbox
            int endtag = 5;  // index to where endtag should be written
            int memsize = 0; // used for allocate memory tag (0x0003000c)
            mp_buffer_info_t bufinfo; // used when an argment is buffer type
            if ((n_args == 2) && (! mp_obj_is_integer(args[1]))) {
                // second arg may be buffer type or list or tuple
                if (mp_get_buffer(args[1], &bufinfo, MP_BUFFER_READ)) {
                    if ((tag == 0x0003000d) || (tag == 0x0003000e) || \
                        (tag == 0x0003000f) || (tag == 0x00030014)) {
                        // copy args[1] to buf as a handle
                        buf[5] = (unsigned int) MP_OBJ_TO_PTR(args[1]);
                        endtag += 1;
                    } else {
                        // copy data pointed from args[1] to buf
                        if (bufinfo.len < buf[3]) {
                            buf[3] = bufinfo.len;
                            memcpy(&buf[5], bufinfo.buf, bufinfo.len);
                        } else {
                            memcpy(&buf[5], bufinfo.buf, buf[3]);
                        }
                        endtag += (buf[3] + 3) >> 2; // +3 for 4 bytes aligned
                    }
                } else if (mp_obj_get_type(args[1]) == &mp_type_list) {
                    // copy all integers in the list to buf
                    mp_obj_list_t *o = (mp_obj_list_t *) args[1];
                    int maxargs = o->len;
                    if (maxargs > buf[3] >> 2) {
                        maxargs = buf[3] >> 2;
                    }
                    for (int i = 0; i < maxargs; i++) {
                        buf[5 + i] = mp_obj_get_int(o->items[i]);
                    }
                    endtag += maxargs;
                } else if (mp_obj_get_type(args[1]) == &mp_type_tuple) {
                    // copy all integers in the tuple to buf
                    mp_obj_tuple_t *o = (mp_obj_tuple_t *) args[1];
                    int maxargs = o->len;
                    if (maxargs > buf[3] >> 2) {
                        maxargs = buf[3] >> 2;
                    }
                    for (int i = 0; i < maxargs; i++) {
                        buf[5 + i] = mp_obj_get_int(o->items[i]);
                    }
                    endtag += maxargs;
                } else {
                    mp_raise_ValueError("Invalid arguments");
                }
            } else {
                // copy all arguments to buf as integer
                int maxargs = n_args - 1;
                if (maxargs > buf[3] >> 2) {
                    maxargs = buf[3] >> 2;
                }
                for (int i = 1; i <= maxargs; i++) {
                    if (mp_obj_is_integer(args[i])) {
                        buf[4 + i] = mp_obj_get_int(args[i]);
                    } else {
                        mp_raise_ValueError("Invalid arguments");
                    }
                }
                endtag += buf[3] >> 2;
            }
            // end tag
            buf[endtag] = 0;

            // keep memsize for later use because buf[5] will be overwritten
            if (tag == 0x0003000c) {
                memsize = buf[5];
            }
/*
            for (int i = 0; i < buf[0] >> 2; i++) {
                printf("%08x ", (unsigned int) buf[i]);
            }
            printf("\n");
*/
            // request to get property through mailbox
            mailbox_write(MB_CH_PROP_ARM, (uint32_t) BUSADDR(buf) >> 4);
            mailbox_read(MB_CH_PROP_ARM);
/*            
            for (int i = 0; i < 5 + (((buf[4]  + 3) & 0x7fffffff) >> 2); i++) {
                printf("%08x ", (unsigned int) buf[i]);
            }
            printf("\n");
*/
            if (buf[1] == MB_PROP_SUCCESS) {
                if (buf[4] & MB_PROP_SUCCESS) {
                    uint32_t datalen = buf[4] & 0x7fffffff;
                    if (datalen == 0) {
                        // no data to return
                        result = mp_const_none;
                    } else if (datalen == 4) {
                        if (tag == 0x0003000c) { // allocate mem
                            result = mp_obj_new_bytearray_by_ref(memsize, (void *) buf[5]);
                        } else if (tag == 0x00030014) { // get dispmanx handle
                            if (buf[5] == 0) {
                                result = mp_obj_new_bytearray_by_ref(bufinfo.len, (void *) buf[6]);
                            } else {
                                mp_raise_OSError(MP_EIO);
                            }
                        } else {
                            // return an int32 value
                            result = mp_obj_new_int(buf[5]);
                        }
                    } else if (tag == 0x00040001) {
                        // return frame buffer or memory block as bytearray
                        result = mp_obj_new_bytearray_by_ref(buf[6], (void *) buf[5]);
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
                    mp_raise_ValueError("Failed getting property value");
                }
            } else {
                // failed getting property values
                mp_raise_ValueError("Mailbox returned 0x80000001");
            }
            m_free(buf);
        }
    }
    return result;
}
MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(gpu_vc_property_obj, 1, 8, gpu_vc_property);
