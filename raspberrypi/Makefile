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

CFLAGS = $(INC) -Wall -std=c99 -nostdlib -marm -mcpu=arm1176jzf-s $(COPT)

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
	uart-qemu.c \
	systimer.c \
	mphalport.c \
	modmcu.c \
	modutime.c \
	modgpu.c \
	modmachine.c \
	machine_pin.c \
	arm_exceptions.c \
	machine_timer.c \


SRC_LIB = \
	lib/utils/stdout_helpers.c \
	lib/utils/interrupt_char.c \
	lib/utils/pyexec.c \
	lib/libc/string0.c \
	lib/libm/math.c \
	lib/libm/fmodf.c \
	lib/libm/nearbyintf.c \
	lib/libm/ef_sqrt.c \
	lib/libm/log1pf.c \
	lib/libm/acoshf.c \
	lib/libm/asinhf.c \
	lib/libm/atanhf.c \
	lib/libm/kf_rem_pio2.c \
	lib/libm/kf_sin.c \
	lib/libm/kf_cos.c \
	lib/libm/kf_tan.c \
	lib/libm/ef_rem_pio2.c \
	lib/libm/erf_lgamma.c \
	lib/libm/sf_sin.c \
	lib/libm/sf_cos.c \
	lib/libm/sf_tan.c \
	lib/libm/sf_frexp.c \
	lib/libm/sf_modf.c \
	lib/libm/sf_ldexp.c \
	lib/libm/sf_erf.c \
	lib/libm/wf_lgamma.c \
	lib/libm/wf_tgamma.c \
	lib/libm/asinfacosf.c \
	lib/libm/atanf.c \
	lib/libm/atan2f.c \
	lib/mp-readline/readline.c \
	lib/timeutils/timeutils.c \

SRC_S = \
    start.s

# USB
ifneq ($(MICROPY_PY_USBHOST),0)
USBHOST_DIR=csud
CSUDLIB=libcsud.a
INC += -I$(USBHOST_DIR)/include
CFLAGS += -DMICROPY_PY_USBHOST=$(MICROPY_PY_USBHOST)
SRC_MOD += usbhost.c \
	usb.c \

LIB += -lcsud
LDFLAGS += -no-wchar-size-warning
USBLIB = $(BUILD)/$(CSUDLIB)

$(USBLIB):
	$(MKDIR) -p $(BUILD)/$(USBHOST_DIR)
	cd $(USBHOST_DIR); $(MAKE) driver CONFIG=DEBUG TYPE=DRIVER TARGET=RPI GNU=$(CROSS_COMPILE) BUILD=../$(BUILD)/$(USBHOST_DIR)/ LIBNAME=../$(BUILD)/$(CSUDLIB) 

endif

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

