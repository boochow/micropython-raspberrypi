#include <stdio.h>
#include <string.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "lib/oofatfs/ff.h"
#include "extmod/vfs_fat.h"

#include "sd.h"
#include "modmachine.h"

/******************************************************************************/
// MicroPython bindings
//
// Expose the SD card as an object with the block protocol.

// there is a singleton SDCard object
const mp_obj_base_t machine_sdcard_obj = {&machine_sdcard_type};

STATIC mp_obj_t machine_sdcard_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // return singleton object
    return (mp_obj_t)&machine_sdcard_obj;
}

STATIC mp_obj_t machine_sdcard_readblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_WRITE);
    mp_uint_t ret = sd_readblock(mp_obj_get_int(block_num), bufinfo.buf, bufinfo.len / SDCARD_BLOCK_SIZE);
    return mp_obj_new_bool(ret == 0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_sdcard_readblocks_obj, machine_sdcard_readblocks);

STATIC mp_obj_t machine_sdcard_writeblocks(mp_obj_t self, mp_obj_t block_num, mp_obj_t buf) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    mp_uint_t ret = sd_writeblock(mp_obj_get_int(block_num), bufinfo.buf, bufinfo.len / SDCARD_BLOCK_SIZE);
    return mp_obj_new_bool(ret == 0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_sdcard_writeblocks_obj, machine_sdcard_writeblocks);

/*
// sector read/write test method
// danger!! this method writes new data to lba=0xbb00
void print_buf(unsigned char *buf) {
    for(int l=0; l < 4; l++) {
        for(int x=0; x < 16; x++) {
            printf("%02x ", buf[l*16+x]);
        }
        printf("  ");
        for(int x=0; x < 16; x++) {
            unsigned char c = buf[l*16+x];
            if ((c < 0x20) || (c == 0x7f)) {
                c = '.';
            } 
            printf("%c", c);
        }
        printf("\n");
    }
    printf("\n");
}

STATIC mp_obj_t machine_sdcard_test(mp_obj_t self) {
    unsigned char bufsave[512];
    unsigned char bufw[512];
    unsigned char bufr[512];
    int ret;
    for(int i = 0; i < 512; i++) {
        bufr[i] = 0;
        bufw[i] = 0x20 + (i % 64);
    }
    ret = sd_readblock(0xbb00, bufsave, 1);
    printf("result=%d\n", ret);
    print_buf(bufsave);

    ret = sd_writeblock(0xbb00, bufw, 1);
    printf("result=%d\n", ret);
    print_buf(bufw);
    ret = sd_readblock(0xbb00, bufr, 1);
    printf("result=%d\n", ret);
    print_buf(bufr);

    ret = sd_writeblock(0xbb00, bufsave, 1);
    printf("result=%d\n", ret);
    ret = sd_readblock(0xbb00, bufr, 1);
    printf("result=%d\n", ret);
    print_buf(bufr);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(machine_sdcard_test_obj, machine_sdcard_test);
*/

STATIC mp_obj_t machine_sdcard_ioctl(mp_obj_t self, mp_obj_t cmd_in, mp_obj_t arg_in) {
    mp_int_t cmd = mp_obj_get_int(cmd_in);
    switch (cmd) {
        case MP_BLOCKDEV_IOCTL_INIT:
            if (sd_init() != SD_OK) {
                return MP_OBJ_NEW_SMALL_INT(-1); // error
            }
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case MP_BLOCKDEV_IOCTL_DEINIT:
            // nothing to do
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case MP_BLOCKDEV_IOCTL_SYNC:
            // nothing to do
            return MP_OBJ_NEW_SMALL_INT(0); // success

        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT:
            // nothing to do
            return MP_OBJ_NEW_SMALL_INT(-1); // error
//            return MP_OBJ_NEW_SMALL_INT(sdcard_get_capacity_in_bytes() / SDCARD_BLOCK_SIZE);

        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE:
            return MP_OBJ_NEW_SMALL_INT(SDCARD_BLOCK_SIZE);

        default: // unknown command
            return MP_OBJ_NEW_SMALL_INT(-1); // error
    }
}

STATIC MP_DEFINE_CONST_FUN_OBJ_3(machine_sdcard_ioctl_obj, machine_sdcard_ioctl);

STATIC const mp_rom_map_elem_t machine_sdcard_locals_dict_table[] = {
    // block device protocol
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&machine_sdcard_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&machine_sdcard_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&machine_sdcard_ioctl_obj) },
//    { MP_ROM_QSTR(MP_QSTR_test), MP_ROM_PTR(&machine_sdcard_test_obj) },
};

STATIC MP_DEFINE_CONST_DICT(machine_sdcard_locals_dict, machine_sdcard_locals_dict_table);

const mp_obj_type_t machine_sdcard_type = {
    { &mp_type_type },
    .name = MP_QSTR_SDCard,
    .make_new = machine_sdcard_make_new,
    .locals_dict = (mp_obj_dict_t*)&machine_sdcard_locals_dict,
};

void sdcard_init_vfs(fs_user_mount_t *vfs, int part) {
    vfs->base.type = &mp_fat_vfs_type;
    vfs->blockdev.flags |= MP_BLOCKDEV_FLAG_HAVE_IOCTL;
    vfs->fatfs.drv = vfs;
    vfs->fatfs.part = part;
    vfs->blockdev.readblocks[0] = MP_OBJ_FROM_PTR(&machine_sdcard_readblocks_obj);
    vfs->blockdev.readblocks[1] = MP_OBJ_FROM_PTR(&machine_sdcard_obj);
// vfs->blockdev.readblocks[2] = MP_OBJ_FROM_PTR(sdcard_read_blocks); // native version
    vfs->blockdev.writeblocks[0] = MP_OBJ_FROM_PTR(&machine_sdcard_writeblocks_obj);
    vfs->blockdev.writeblocks[1] = MP_OBJ_FROM_PTR(&machine_sdcard_obj);
//  vfs->blockdev.writeblocks[2] = MP_OBJ_FROM_PTR(sdcard_write_blocks); // native version
    vfs->blockdev.u.ioctl[0] = MP_OBJ_FROM_PTR(&machine_sdcard_ioctl_obj);
    vfs->blockdev.u.ioctl[1] = MP_OBJ_FROM_PTR(&machine_sdcard_obj);
}

