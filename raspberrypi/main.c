#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/compile.h"
#include "py/runtime.h"
#include "py/stackctrl.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/mperrno.h"
#include "lib/utils/interrupt_char.h"
#include "lib/utils/pyexec.h"

#include "arm_exceptions.h"
#include "rpi.h"
#include "mphalport.h"
#include "usbhost.h"

#if MICROPY_MOUNT_SD_CARD

#include "lib/oofatfs/ff.h"
#include "extmod/vfs_fat.h"
#include "sd.h"
#include "modmachine.h"

extern void sdcard_init_vfs(fs_user_mount_t *vfs, int part);

#endif

void clear_bss(void) {
    extern void * _bss_start;
    extern void *  _bss_end;
    unsigned int *p;

    for(p = (unsigned int *)&_bss_start; p < (unsigned int *) &_bss_end; p++) {
        *p = 0;
    }
}

#define TAG_CMDLINE 0x54410009
char *arm_boot_tag_cmdline(const int32_t *ptr) {
    int32_t datalen = *ptr;
    int32_t tag = *(ptr + 1);

    if (ptr != 0) {
        while(tag) {
            if (tag == TAG_CMDLINE) {
                return (char *) (ptr + 2);
            } else {
                ptr += datalen;
                datalen = *ptr;
                tag = *(ptr + 1);
            }
        }
    }
    return NULL;
}

#if MICROPY_MOUNT_SD_CARD

STATIC bool init_sdcard_fs(void) {
    bool first_part = true;
    for (int part_num = 1; part_num <= 4; ++part_num) {
        // create vfs object
        fs_user_mount_t *vfs_fat = m_new_obj_maybe(fs_user_mount_t);
        mp_vfs_mount_t *vfs = m_new_obj_maybe(mp_vfs_mount_t);
        if (vfs == NULL || vfs_fat == NULL) {
            printf("vfs=NULL\n");
            break;
        }
        vfs_fat->blockdev.flags = MP_BLOCKDEV_FLAG_FREE_OBJ;
        sdcard_init_vfs(vfs_fat, part_num);

        // try to mount the partition
        FRESULT res = f_mount(&vfs_fat->fatfs);

        if (res != FR_OK) {
            // couldn't mount
            m_del_obj(fs_user_mount_t, vfs_fat);
            m_del_obj(mp_vfs_mount_t, vfs);
        } else {
            // mounted via FatFs, now mount the SD partition in the VFS
            if (first_part) {
                // the first available partition is traditionally called "sd" for simplicity
                vfs->str = "/sd";
                vfs->len = 3;
            } else {
                // subsequent partitions are numbered by their index in the partition table
                if (part_num == 2) {
                    vfs->str = "/sd2";
                } else if (part_num == 2) {
                    vfs->str = "/sd3";
                } else {
                    vfs->str = "/sd4";
                }
                vfs->len = 4;
            }
            vfs->obj = MP_OBJ_FROM_PTR(vfs_fat);
            vfs->next = NULL;
            for (mp_vfs_mount_t **m = &MP_STATE_VM(vfs_mount_table);; m = &(*m)->next) {
                if (*m == NULL) {
                    *m = vfs;
                    break;
                }
            }

            if (first_part) {
                // use SD card as current directory
                MP_STATE_PORT(vfs_cur) = vfs;
            }
            first_part = false;
        }
    }

    if (first_part) {
        printf("PYB: can't mount SD card\n");
        return false;
    } else {
        return true;
    }
}

#endif

int arm_main(uint32_t r0, uint32_t id, const int32_t *atag) {
    bool use_qemu = false;
    
    extern char * _heap_end;
    extern char * _heap_start;
    extern char * _estack;

    clear_bss();
    mp_stack_set_top(&_estack);
    mp_stack_set_limit((char*)&_estack - (char*)&_heap_end - 1024);

    // check ARM boot tag and set up standard I/O
    if (atag != (int32_t *) 0x100) {
        // atag points to 0x100 in general, so something may be wrong?
        atag = (int32_t *) 0x100;
    }
    if (atag && (strcmp("qemu", arm_boot_tag_cmdline(atag)) == 0)) {
        use_qemu = true;
    }

    if (use_qemu) {
        uart_init(UART_QEMU);
    } else {
        uart_init(MINI_UART);
        arm_irq_disable();
        arm_exceptions_init();
        arm_irq_enable();
        rpi_init();
    }

    // start MicroPython
    while (true) {
        gc_init (&_heap_start, &_heap_end );
        mp_init();
        mp_obj_list_init(mp_sys_path, 0);
        mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_)); // current dir (or base dir of the script)
        mp_obj_list_init(mp_sys_argv, 0);
        mp_hal_stdout_tx_strn("\r\n", 2);

#if MICROPY_MODULE_FROZEN_MPY
        pyexec_frozen_module("_boot.py");
#endif

#if MICROPY_MOUNT_SD_CARD
        bool mounted_sdcard = false;
        if (!use_qemu) {
            printf("mounting SD card...");
            mounted_sdcard = init_sdcard_fs();
            if (mounted_sdcard) {
                printf("done\n\r");
            } else {
                printf("failed\n\r");
            }
        }

        if (!use_qemu && mounted_sdcard) {
            mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_sd_slash_lib));
            mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR__slash_sd));
            if (pyexec_mode_kind == PYEXEC_MODE_FRIENDLY_REPL) {
                mp_import_stat_t stat;
                // Run the boot config script from the current directory.
                const char boot_py[] = "boot.py";
                stat = mp_import_stat(boot_py);
                if (stat == MP_IMPORT_STAT_FILE) {
                    int ret = pyexec_file(boot_py);
                    if (!ret) {
                        printf("%s: execution error\n\r", boot_py);
                    }
                }
                // Run the main script from the current directory.
                const char main_py[] = "main.py";
                stat = mp_import_stat(main_py);
                if (stat == MP_IMPORT_STAT_FILE) {
                    int ret = pyexec_file(main_py);
                    if (!ret) {
                        printf("%s: execution error\n\r", main_py);
                    }
                }
            }
        }
#endif

#ifdef MICROPY_HW_USBHOST
        // USB host library initialization must be called after MicroPython's
        // initialization because USB host library allocates its memory blocks
        // using MemoryAllocate in usbhost.c which in turn calls m_alloc().
        if (!use_qemu) {
            rpi_usb_host_init();
            usbkbd_setup();
        }
#endif

        for (;;) {
            if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) {
                if (pyexec_raw_repl() != 0) {
                    break;
                }
            } else {
                if (pyexec_friendly_repl() != 0) {
                    break;
                }
            }
        }
#ifdef MICROPY_HW_USBHOST
        if (!use_qemu) {
            rpi_usb_host_deinit();
        }
#endif
        mp_deinit();
        printf("PYB: soft reboot\n");
    }
    return 0;
}

void nlr_jump_fail(void *val) {
    while (1);
}

void NORETURN __fatal_error(const char *msg) {
    while (1);
}


#ifdef NDEBUG
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    __fatal_error("Assertion failed");
}
#else
void MP_WEAK __assert_func(const char *file, int line, const char *func, const char *expr) {
    printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif

