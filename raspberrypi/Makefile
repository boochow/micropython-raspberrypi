BOARD ?= RPI1

include ../micropython/py/mkenv.mk
include mpconfigport.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

# include py core make definitions
include $(TOP)/py/py.mk

CROSS_COMPILE = arm-none-eabi-

OBJCPY=$(CROSS_COMPILE)objcopy


INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

# compiler settings
CWARN = -Wall -Werror
CWARN += -Wpointer-arith -Wuninitialized
CFLAGS = $(INC) $(CWARN) -std=c99 -nostdlib -marm -mcpu=arm1176jzf-s $(CFLAGS_MOD) $(COPT)

#Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -ggdb
else
CFLAGS += -Os -DNDEBUG
endif

CCVERSION:=$(shell $(CC) --version | sed 's/[ ]/\n/g' | grep "\." | xargs | awk '{ print $$1 }')
ARMGCCLIBPATH=/usr/lib/gcc/arm-none-eabi/$(CCVERSION)

LDFLAGS = -nostdlib -T kernel.ld -Map=$@.map --cref
LIBS = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)

SRC_LIB = $(addprefix lib/,\
	libc/string0.c )

SRC_C = \
	main.c \
	uart.c \
	rpi.c \
	mphalport.c \
	modmcu.c \
	modutime.c \
	modgpu.c \
	modmachine.c \
	machine_pin.c \
	arm_exceptions.c \
	arm_ex_handler_weak.c \
	machine_timer.c \
	bcm283x_mailbox.c \
	gpu.c \
	gccollect.c \
	mini-uart.c \
	moduos.c \
	fatfs_port.c \


SRC_LIB = $(addprefix lib/,\
	utils/stdout_helpers.c \
	utils/interrupt_char.c \
	utils/pyexec.c \
	libc/string0.c \
	libm/math.c \
	libm/fmodf.c \
	libm/nearbyintf.c \
	libm/ef_sqrt.c \
	libm/log1pf.c \
	libm/acoshf.c \
	libm/asinhf.c \
	libm/atanhf.c \
	libm/kf_rem_pio2.c \
	libm/kf_sin.c \
	libm/kf_cos.c \
	libm/kf_tan.c \
	libm/ef_rem_pio2.c \
	libm/erf_lgamma.c \
	libm/sf_sin.c \
	libm/sf_cos.c \
	libm/sf_tan.c \
	libm/sf_frexp.c \
	libm/sf_modf.c \
	libm/sf_ldexp.c \
	libm/sf_erf.c \
	libm/wf_lgamma.c \
	libm/wf_tgamma.c \
	libm/asinfacosf.c \
	libm/atanf.c \
	libm/atan2f.c \
	mp-readline/readline.c \
	timeutils/timeutils.c \
	)

SRC_S = \
    start.s \
    gchelper.s \

# USB
ifneq ($(MICROPY_PY_USBHOST),0)
USBHOST_DIR=csud
CSUDLIB=libcsud.a
INC += -I$(USBHOST_DIR)/include
CFLAGS += -DMICROPY_PY_USBHOST=$(MICROPY_PY_USBHOST)
SRC_MOD += usbhost.c \
	machine_usb_mode.c \

LIB += -lcsud
LDFLAGS += -no-wchar-size-warning
USBLIB = $(BUILD)/$(CSUDLIB)

$(USBLIB): 
	$(ECHO) "MAKE $@"
	$(Q)$(MKDIR) -p $(BUILD)/$(USBHOST_DIR)
	$(Q)$(MAKE) -s -C $(USBHOST_DIR) driver CONFIG=FINAL TYPE=DRIVER TARGET=RPI GNU=$(CROSS_COMPILE) BUILD=../$(BUILD)/$(USBHOST_DIR)/ LIBNAME=../$(USBLIB)

endif

# FatFS VFS support
SRC_LIB += $(addprefix lib/,\
	oofatfs/ff.c \
	oofatfs/option/unicode.c \
	)

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_MOD) $(SRC_LIB)

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_MOD:.c=.o) $(SRC_S:.s=.o) $(SRC_LIB:.c=.o))

all: $(BUILD)/firmware.img

list: $(BUILD)/firmware.elf
	$(CROSS_COMPILE)objdump -d $(BUILD)/firmware.elf > $(BUILD)/firmware.list

$(BUILD)/firmware.elf: $(OBJ) $(USBLIB)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.img: $(BUILD)/firmware.elf
	$(OBJCPY) $(BUILD)/firmware.elf -O binary $(BUILD)/firmware.img

$(BUILD)/config.txt: config-template.txt $(BUILD)/firmware.img 
	$(RM) -f $@
	$(CP) $< $@
	$(ECHO) "kernel=firmware.img" >> $@

$(BUILD)/firmware-qemu.img: $(OBJ) $(USBLIB)
	$(ECHO) "LINK $@"
	$(Q)$(LD) --defsym=_load_addr=0x10000 $(LDFLAGS) -o $@.elf $^ $(LIBS)
	$(OBJCPY) $@.elf -O binary $@

run: $(BUILD)/firmware-qemu.img
	@qemu-system-arm \
	-kernel $< \
	-append "qemu" \
	-cpu arm1176 \
	-M versatilepb \
	-m 512 \
	-no-reboot \
	-nographic \
	-monitor null \
	-serial stdio \
	-redir tcp:10022::22

include $(TOP)/py/mkrules.mk

