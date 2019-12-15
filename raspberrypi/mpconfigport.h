#include <stdint.h>
#include "arm_exceptions.h"
#include "usbhost.h"

// options to control how MicroPython is built
#define MICROPY_DEBUG_VERBOSE       (0)
#define MICROPY_QSTR_BYTES_IN_HASH  (1)
#define MICROPY_ALLOC_PATH_MAX      (512)
#define MICROPY_ALLOC_PARSE_CHUNK_INIT (16)
#define MICROPY_EMIT_X64            (0)
#define MICROPY_EMIT_X86            (0)
#define MICROPY_EMIT_THUMB          (0)
#define MICROPY_EMIT_ARM            (0)
#define MICROPY_EMIT_XTENSA         (0)
#define MICROPY_EMIT_INLINE_THUMB   (0)
#define MICROPY_EMIT_INLINE_THUMB_ARMV7M (0)
#define MICROPY_EMIT_INLINE_THUMB_FLOAT (0)
#define MICROPY_COMP_MODULE_CONST   (1)
#define MICROPY_COMP_CONST          (1)
#define MICROPY_COMP_DOUBLE_TUPLE_ASSIGN (0)
#define MICROPY_COMP_TRIPLE_TUPLE_ASSIGN (0)
#define MICROPY_STACK_CHECK         (1)
#define MICROPY_MEM_STATS           (0)
#define MICROPY_DEBUG_PRINTERS      (0)
#define MICROPY_READER_VFS          (MICROPY_VFS)
#define MICROPY_ENABLE_GC           (1)
#define MICROPY_ENABLE_FINALISER    (1)
#define MICROPY_HELPER_REPL         (1)
#define MICROPY_REPL_EMACS_KEYS     (1)
#define MICROPY_REPL_AUTO_INDENT    (1)
#define MICROPY_ENABLE_SOURCE_LINE  (1)
#define MICROPY_ENABLE_DOC_STRING   (1)
#define MICROPY_ERROR_REPORTING     (MICROPY_ERROR_REPORTING_TERSE)
#define MICROPY_BUILTIN_METHOD_CHECK_SELF_ARG (1)
#define MICROPY_PY_ASYNC_AWAIT      (1)
#define MICROPY_USE_INTERNAL_ERRNO  (1)
#define MICROPY_PY_DESCRIPTORS      (1)
#define MICROPY_PY_ALL_SPECIAL_METHODS (1)
#define MICROPY_PY_BUILTINS_STR_UNICODE (1)
#define MICROPY_PY_BUILTINS_BYTEARRAY (1)
#define MICROPY_PY_BUILTINS_MEMORYVIEW (1)
#define MICROPY_PY_BUILTINS_ENUMERATE (1)
#define MICROPY_PY_BUILTINS_FROZENSET (1)
#define MICROPY_PY_BUILTINS_REVERSED (1)
#define MICROPY_PY_BUILTINS_SET     (1)
#define MICROPY_PY_BUILTINS_SLICE   (1)
#define MICROPY_PY_BUILTINS_PROPERTY (1)
#define MICROPY_PY_BUILTINS_HELP    (1)
//#define MICROPY_PY_BUILTINS_HELP_TEXT rpi_help_text
#define MICROPY_PY_BUILTINS_HELP_MODULES (1)
#define MICROPY_PY___FILE__         (1)
#define MICROPY_PY_GC               (1)
#define MICROPY_PY_ARRAY            (1)
#define MICROPY_PY_ARRAY_SLICE_ASSIGN (1)
#define MICROPY_PY_ATTRTUPLE        (1)
#define MICROPY_PY_COLLECTIONS      (1)
#define MICROPY_PY_COLLECTIONS_DEQUE (1)
#define MICROPY_PY_COLLECTIONS_ORDEREDDICT (1)
#define MICROPY_PY_CMATH            (1)
#define MICROPY_PY_IO               (1)
#define MICROPY_PY_IO_IOBASE        (1)
#define MICROPY_PY_IO_FILEIO        (MICROPY_VFS_FAT) // because mp_type_fileio/textio point to fatfs impl
#define MICROPY_PY_STRUCT           (1)
#define MICROPY_PY_SYS              (1)
#define MICROPY_PY_SYS_MAXSIZE      (1)
#define MICROPY_PY_SYS_EXIT         (1)
#define MICROPY_PY_SYS_STDFILES     (1)
#define MICROPY_PY_SYS_STDIO_BUFFER (1)
#define MICROPY_PY_UERRNO           (1)
#define MICROPY_PY_UBINASCII        (1)
#define MICROPY_PY_UCTYPES          (1)
#define MICROPY_PY_UHASHLIB         (1)
#define MICROPY_PY_UHEAPQ           (1)
#define MICROPY_PY_UTIMEQ           (1)
#define MICROPY_PY_UJSON            (1)
#define MICROPY_PY_URANDOM          (1)
#define MICROPY_PY_URE              (1)
#define MICROPY_PY_URE_SUB          (1)
#define MICROPY_PY_SYS_EXC_INFO     (1)
#define MICROPY_ENABLE_EMERGENCY_EXCEPTION_BUF   (1)
#define MICROPY_EMERGENCY_EXCEPTION_BUF_SIZE  (256)
#define MICROPY_PY_MACHINE          (1)
#define MICROPY_PY_UTIME_MP_HAL     (1)
#define MICROPY_PY_UZLIB            (1)
#define MICROPY_PY_FRAMEBUF         (1)
#define MICROPY_PY_VFS              (1)
#define MICROPY_PY_MICROPYTHON_MEM_INFO (1)
#define MICROPY_PY_OS_DUPTERM       (1)
#define MICROPY_CPYTHON_COMPAT      (1)
#define MICROPY_LONGINT_IMPL        (MICROPY_LONGINT_IMPL_MPZ)
#define MICROPY_FLOAT_IMPL          (MICROPY_FLOAT_IMPL_FLOAT)
#define MICROPY_MODULE_WEAK_LINKS   (1)
#define MICROPY_USE_INTERNAL_PRINTF (1)
#define MICROPY_ENABLE_SCHEDULER    (1)
#define MICROPY_SCHEDULER_DEPTH     (8)
#define MICROPY_STREAMS_NON_BLOCK   (1)

#define MICROPY_KBD_EXCEPTION       (1)

#define MICROPY_VFS                 (1)
#define MICROPY_FATFS_ENABLE_LFN    (1)
#define MICROPY_FATFS_RPATH         (2)
#define MICROPY_FATFS_MAX_SS        (4096)
#define MICROPY_FATFS_LFN_CODE_PAGE 437 /* 1=SFN/ANSI 437=LFN/U.S.(OEM) */
#define MICROPY_VFS_FAT             (1)
#define MICROPY_FATFS_MULTI_PARTITION  (1)

// type definitions for the specific machine

#define malloc(n) m_malloc(n)
#define free(p) m_free(p)
#define realloc(p, n) m_realloc(p, n)

#define MICROPY_MAKE_POINTER_CALLABLE(p) ((void*)((mp_uint_t)(p) | 1))

#define MP_SSIZE_MAX (0x7fffffff)

#define UINT_FMT "%u"
#define INT_FMT "%d"

typedef int mp_int_t; // must be pointer size
typedef unsigned mp_uint_t; // must be pointer size
typedef long mp_off_t;

// dummy print
#define MP_PLAT_PRINT_STRN(str, len) mp_hal_stdout_tx_strn_cooked(str, len)

extern const struct _mp_obj_module_t mcu_module;
extern const struct _mp_obj_module_t utime_module;
extern const struct _mp_obj_module_t gpu_module;
extern const struct _mp_obj_module_t mp_module_uos;

#define mp_type_fileio mp_type_vfs_fat_fileio
#define mp_type_textio mp_type_vfs_fat_textio

// use vfs's functions for import stat and builtin open
#define mp_import_stat mp_vfs_import_stat
#define mp_builtin_open mp_vfs_open
#define mp_builtin_open_obj mp_vfs_open_obj

// extra built in names to add to the global namespace
#define MICROPY_PORT_BUILTINS \
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&mp_builtin_open_obj) },

#define MICROPY_PORT_BUILTIN_MODULES                       \
    { MP_ROM_QSTR(MP_QSTR_mcu), MP_ROM_PTR(&mcu_module) }, \
    { MP_ROM_QSTR(MP_QSTR_utime), MP_ROM_PTR(&utime_module) }, \
    { MP_ROM_QSTR(MP_QSTR_gpu), MP_ROM_PTR(&gpu_module) }, \
    { MP_ROM_QSTR(MP_QSTR_machine), MP_ROM_PTR(&mp_module_machine) }, \
    { MP_ROM_QSTR(MP_QSTR_uos), MP_ROM_PTR(&mp_module_uos) }, \

#define MICROPY_PORT_BUILTIN_MODULE_WEAK_LINKS \
    { MP_ROM_QSTR(MP_QSTR_binascii), MP_ROM_PTR(&mp_module_ubinascii) }, \
    { MP_ROM_QSTR(MP_QSTR_collections), MP_ROM_PTR(&mp_module_collections) }, \
    { MP_ROM_QSTR(MP_QSTR_errno), MP_ROM_PTR(&mp_module_uerrno) }, \
    { MP_ROM_QSTR(MP_QSTR_hashlib), MP_ROM_PTR(&mp_module_uhashlib) }, \
    { MP_ROM_QSTR(MP_QSTR_io), MP_ROM_PTR(&mp_module_io) }, \
    { MP_ROM_QSTR(MP_QSTR_json), MP_ROM_PTR(&mp_module_ujson) }, \
    { MP_ROM_QSTR(MP_QSTR_os), MP_ROM_PTR(&mp_module_uos) }, \
    { MP_ROM_QSTR(MP_QSTR_random), MP_ROM_PTR(&mp_module_urandom) }, \
    { MP_ROM_QSTR(MP_QSTR_re), MP_ROM_PTR(&mp_module_ure) }, \
    { MP_ROM_QSTR(MP_QSTR_struct), MP_ROM_PTR(&mp_module_ustruct) }, \
    { MP_ROM_QSTR(MP_QSTR_time), MP_ROM_PTR(&utime_module) }, \
    { MP_ROM_QSTR(MP_QSTR_zlib), MP_ROM_PTR(&mp_module_uzlib) }, \

// Raspberry Pi CPU(ARMv6) can enable/disable all irqs by setting
// bit 7 of CPSR so we do not need to save/restore irq flags state.

__attribute__(( always_inline )) static inline void enable_irq(mp_uint_t state) {
    arm_irq_enable();
}

__attribute__(( always_inline )) static inline mp_uint_t disable_irq(void) {
    arm_irq_disable();
    return 0x80;
}

//#define MICROPY_BEGIN_ATOMIC_SECTION()     disable_irq()
//#define MICROPY_END_ATOMIC_SECTION(state)  enable_irq(state)

// We need to provide a declaration/definition of alloca()
#include <alloca.h>

#ifdef RPI1
#define MICROPY_HW_BOARD_NAME        "Raspberry Pi"
#define MICROPY_HW_MCU_NAME          "ARM1176JZF-S"
#else
#define MICROPY_HW_BOARD_NAME        "Raspberry Pi2"
#define MICROPY_HW_MCU_NAME          "Cortex-A7"
#endif

#define MICROPY_PY_SYS_PLATFORM      "raspberrypi"

#define MP_STATE_PORT MP_STATE_VM

#define MICROPY_PORT_ROOT_POINTERS \
    const char *readline_hist[8]; \
    hcd_globals_t *hcd_globals; \

