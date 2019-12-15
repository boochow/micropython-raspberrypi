# BOARD: RPI1 or RPI2
BOARD ?= RPI1

include ../micropython/py/mkenv.mk
include mpconfigport.mk

# qstr definitions (must come before including py.mk)
QSTR_DEFS = qstrdefsport.h

FROZEN_DIR ?= scripts
# FROZEN_MPY_DIR ?= modules

# include py core make definitions
include $(TOP)/py/py.mk

CROSS_COMPILE = arm-none-eabi-

OBJCPY=$(CROSS_COMPILE)objcopy

ZIP = /usr/bin/zip
RELEASE=`/bin/date +%Y%m%d`-rpi
RELFILES=fs/

INC += -I.
INC += -I$(TOP)
INC += -I$(BUILD)

# compiler settings
CWARN = -Wall -Werror
CWARN += -Wpointer-arith -Wuninitialized

ifeq ($(BOARD),RPI1)
	CFLAGS_RPI = -marm -mcpu=arm1176jzf-s -mno-unaligned-access -DRPI1
	CFLAGS_FPU = -mfloat-abi=hard -mfpu=vfp
	ASFLAGS = --defsym RPI1=1
else ifeq ($(BOARD),RPI2)
	CFLAGS_RPI = -marm -mcpu=cortex-a7 -mno-unaligned-access
	CFLAGS_FPU = -mfloat-abi=hard -mfpu=neon-vfpv4
endif

CFLAGS_RPI += $(CFLAGS_FPU) -fsingle-precision-constant -Wdouble-promotion
CFLAGS = $(INC) $(CWARN) -std=c99 -nostdlib $(CFLAGS_RPI) $(CFLAGS_MOD) 

ASFLAGS += $(CFLAGS_FPU)

#Debugging/Optimization
ifeq ($(DEBUG), 1)
CFLAGS += -O0 -gdwarf-2
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
	bcm283x_gpio.c \
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
	sd.c \
	machine_sdcard.c \
	i2c.c \
	machine_i2c.c \
	spi.c \
	machine_spi.c \
	vc_property.c \
	gpu_vc_property.c \
	bcm283x_clockmgr.c \
	pwm.c \
	machine_pwm.c \
	machine_clockmgr.c \

SRC_LIB = $(addprefix lib/,\
	utils/stdout_helpers.c \
	utils/sys_stdio_mphal.c \
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
ifneq ($(MICROPY_HW_USBHOST),0)
RELEASE :=$(RELEASE)-usb
USBHOST_DIR=csud
CSUDLIB=libcsud.a
INC += -I$(USBHOST_DIR)/include
CFLAGS += -DMICROPY_HW_USBHOST=$(MICROPY_HW_USBHOST)
SRC_MOD += usbhost.c \
	machine_usb_mode.c \
	usbkbd.c \

LIB += -lcsud
LDFLAGS += -no-wchar-size-warning
USBLIB = $(BUILD)/$(CSUDLIB)

$(USBLIB): 
	$(ECHO) "MAKE $@"
	$(Q)$(MKDIR) -p $(BUILD)/$(USBHOST_DIR)
	$(Q)$(MAKE) -s -C $(USBHOST_DIR) driver CONFIG=FINAL TYPE=DRIVER TARGET=$(BOARD) GNU=$(CROSS_COMPILE) BUILD=../$(BUILD)/$(USBHOST_DIR)/ LIBNAME=../$(USBLIB) COPT="-Os $(CFLAGS_FPU)"

endif

# FatFS VFS support
SRC_LIB += $(addprefix lib/,\
	oofatfs/ff.c \
	oofatfs/ffunicode.c \
	)
CFLAGS += -DMICROPY_MOUNT_SD_CARD=$(MICROPY_MOUNT_SD_CARD)

# List of sources for qstr extraction
SRC_QSTR += $(SRC_C) $(SRC_MOD) $(SRC_LIB)

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_MOD:.c=.o) $(SRC_S:.s=.o) $(SRC_LIB:.c=.o))


ifneq ($(FROZEN_DIR),)
# To use frozen source modules, put your .py files in a subdirectory (eg scripts/)
# and then invoke make with FROZEN_DIR=scripts (be sure to build from scratch).
CFLAGS += -DMICROPY_MODULE_FROZEN_STR
endif

ifneq ($(FROZEN_MPY_DIR),)
# To use frozen bytecode, put your .py files in a subdirectory (eg frozen/) and
# then invoke make with FROZEN_MPY_DIR=frozen (be sure to build from scratch).
CFLAGS += -DMICROPY_QSTR_EXTRA_POOL=mp_qstr_frozen_const_pool
CFLAGS += -DMICROPY_MODULE_FROZEN_MPY
endif

.PHONY: qemu


all: $(BUILD)/firmware.img $(BUILD)/config.txt

list: $(BUILD)/firmware.elf
	$(CROSS_COMPILE)objdump -d $(BUILD)/firmware.elf > $(BUILD)/firmware.list

$(BUILD)/firmware.elf: $(OBJ) $(USBLIB)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.img: $(BUILD)/firmware.elf
	$(OBJCPY) $(BUILD)/firmware.elf -O binary $(BUILD)/firmware.img

$(BUILD)/config.txt: config-template.txt
	$(ECHO) "GEN $@"
	$(Q)$(RM) -f $@
	$(Q)$(CP) $< $@
	$(Q)$(ECHO) "kernel=firmware.img" >> $@

release: $(BUILD)/firmware.img $(BUILD)/config.txt
	$(ECHO) "GEN $(BUILD)/$(RELEASE).zip"
	$(Q)$(ZIP) $(BUILD)/$(RELEASE) -jq $^
	$(Q)cd $(RELFILES); $(ZIP) ../$(BUILD)/$(RELEASE) -rq .

$(BUILD)/firmware-qemu.img: $(OBJ) $(USBLIB)
	$(ECHO) "LINK $@"
	$(Q)$(LD) --defsym=_load_addr=0x10000 $(LDFLAGS) -o $@.elf $^ $(LIBS)
	$(OBJCPY) $@.elf -O binary $@

ifeq ($(BOARD),RPI1)
qemu: $(BUILD)/firmware-qemu.img
	@./qemu-system-arm \
	-kernel $< \
	-append "qemu" \
	-cpu arm1176 \
	-M versatilepb \
	-m 512 \
	-no-reboot \
	-nographic \
	-monitor null \
	-serial stdio

else ifeq ($(BOARD),RPI2)
qemu: $(BUILD)/firmware-qemu.img
	@./qemu-system-arm \
	-kernel $< \
	-append "qemu" \
	-M raspi2 \
	-cpu arm1176 \
	-m 1G \
	-no-reboot \
	-nographic \
	-monitor null \
	-smp 4 \
	-serial stdio

endif

include $(TOP)/py/mkrules.mk

$(BUILD)/%.o: %.s
	$(ECHO) "AS $<"
	$(Q)$(AS) $(ASFLAGS) -o $@ $<
