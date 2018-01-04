include ../micropython/py/mkenv.mk

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
	gpio.c \
	mphalport.c \
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

SRC_S = \
    start.s

OBJ = $(PY_O) $(addprefix $(BUILD)/, $(SRC_C:.c=.o) $(SRC_S:.s=.o) $(SRC_LIB:.c=.o))

all: $(BUILD)/firmware.img

list: $(BUILD)/firmware.elf
	$(CROSS_COMPILE)objdump -d $(BUILD)/firmware.elf > $(BUILD)/firmware.list

$(BUILD)/firmware.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(Q)$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)
	$(Q)$(SIZE) $@

$(BUILD)/firmware.img: $(BUILD)/firmware.elf
	$(OBJCPY) $(BUILD)/firmware.elf -O binary $(BUILD)/firmware.img

$(BUILD)/config.txt: config-template.txt $(BUILD)/firmware.img 
	$(RM) -f $@
	$(CP) $< $@
	$(ECHO) "kernel=firmware.img" >> $@

run: $(BUILD)/firmware.elf
	@qemu-system-arm \
	-kernel $< \
	-cpu arm1176 \
	-M versatilepb \
	-m 256 \
	-no-reboot \
	-nographic \
	-monitor null \
	-serial stdio \
	-redir tcp:10022::22

include $(TOP)/py/mkrules.mk
