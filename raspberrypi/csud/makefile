###############################################################################
#	makefile
#	 by Alex Chadwick
#
#	A makefile script for generation of CSUD.
###############################################################################

# Default parameters
TYPE ?= LOWLEVEL
TARGET ?= NONE
CONFIG ?= FINAL

# The intermediate directory for compiled object files.
BUILD ?= build/

# The directory in which source files are stored.
SOURCE ?= source/

# The directory in which configuration files are stored.
CONFIGDIR ?= configuration/

# The name of the output file to generate.
LIBNAME ?= libcsud.a

# The include directory
INCDIR ?= include/

all:
	@echo "CUSD - Chadderz Simple USB Driver"
	@echo "	by Alex Chadwick"
	@echo "Usage: make driver CONFIG=config TYPE=type TARGET=target GNU=gnu"
	@echo "Parameters:"
	@echo " config - DEBUG or FINAL (default)"
	@echo "          alters amount of messages, checks, and the speed."
	@echo " type   - STANDALONE, LOWLEVEL (default), or DRIVER"
	@echo "          alters how complete the driver is STANDALONE for no external"
	@echo "          dependencies LOWLEVEL for only key dependencies, DRIVER for" 
	@echo "          typical levels."
	@echo " target - RPI, NONE (default)"
	@echo "          alters the target system. NONE for dummy driver, RPI for" 
	@echo "          the RaspberryPi"
	@echo " gnu    - A gnu compiler prefix (arm-none-eabi-) or empty (default)."
	@echo "          The compiler chain to use (for cross compiling)."
	@echo "See arguments for more."

# The flags to pass to GCC for compiling.
# -std=c99: Use c99 standard.
# -fpack-struct: Do not insert unnecessary padding in structures to make them faster.
# -Wno-packet-bitfield-compat: Do not warn about structures which would complie differently on older gcc.
# -fshort-wchar: Treat wchar as a 16 bit value.
# -Wall: Print lots of compiler warnings
CFLAGS += -std=c99 -fpack-struct -Wno-packed-bitfield-compat -fshort-wchar -Wall 
CFLAGS += $(patsubst %,-I%,$(INCDIRS)) 
	
include $(CONFIGDIR)makefile.in
include $(SOURCE)makefile.in

# Rule to make everything.
driver: $(LIBNAME) 
	
# Rule to make the driver file.
$(LIBNAME) : $(patsubst %/,%, $(BUILD)) $(OBJECTS)
	-rm -f $(LIBNAME)
	$(GNU)ar rc $(LIBNAME) $(OBJECTS)
		
# Rule to make the c object files.
GCC := $(GNU)gcc $(CFLAGS) -c -I$(INCDIR)

$(BUILD):
	mkdir $(patsubst %/,%, $(BUILD))
	
# Rule to clean files.
clean : 
	-rm -f $(wildcard $(BUILD)*.*)
	-rm -f $(LIBNAME)

.PHONY: clean driver all
