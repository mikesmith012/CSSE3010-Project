# Set mylib folder path.
MYLIB_PATH=$(SOURCELIB_ROOT)/../repo/mylib

# Set folder path with header files to include.
CFLAGS += -I$(MYLIB_PATH)

# List all c files locations that must be included
LIBSRCS += $(MYLIB_PATH)/s4640878_joystick.c 
LIBSRCS += $(MYLIB_PATH)/s4640878_lta1000g.c