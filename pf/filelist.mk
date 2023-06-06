# Set mylib folder path.
MYLIB_PATH=$(SOURCELIB_ROOT)/../repo/mylib

# Set folder path with header files to include.
CFLAGS += -I$(MYLIB_PATH)

# List all c files locations that must be included
LIBSRCS += $(MYLIB_PATH)/s4640878_joystick.c 
LIBSRCS += $(MYLIB_PATH)/s4640878_lta1000g.c 
LIBSRCS += $(MYLIB_PATH)/s4640878_pantilt.c
LIBSRCS += $(MYLIB_PATH)/s4640878_hamming.c 
LIBSRCS += $(MYLIB_PATH)/s4640878_irremote.c 
LIBSRCS += $(MYLIB_PATH)/s4640878_oled.c

LIBSRCS += $(MYLIB_PATH)/s4640878_CAG_simulator.c 
LIBSRCS += $(MYLIB_PATH)/s4640878_CAG_display.c
LIBSRCS += $(MYLIB_PATH)/s4640878_CAG_grid.c
LIBSRCS += $(MYLIB_PATH)/s4640878_cli_task.c
LIBSRCS += $(MYLIB_PATH)/s4640878_cli_CAG_mnemonic.c
LIBSRCS += $(MYLIB_PATH)/s4640878_CAG_joystick.c

# Including memory heap model
LIBSRCS += $(FREERTOS_PATH)/portable/MemMang/heap_3.c