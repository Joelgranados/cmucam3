include defs.mk

CSOURCES=interrupt.c cc3.c cc3_hal.c syscalls.c serial.c \
	servo.c rdcf2.c mmc_init.c mmc_hardware.c mmc_module.c
INCLUDES=cc3_hal.h devices.h ioctl.h lpc_config.h rdcf2.h servo.h \
	cc3_pin_defines.h interrupt.h LPC2100.h mmc_hardware.h serial.h spi.h \
	mmc_ioctl.h
ASMSOURCES=startup.s


COBJS=$(patsubst %.c, %$(THUMB_SUFFIX).o,$(CSOURCES))
ASMOBJS=$(patsubst %.s, %$(THUMB_SUFFIX).o,$(ASMSOURCES))

all: $(HALLIB)

$(ASMOBJS): %$(THUMB_SUFFIX).o : %.s $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS)

$(COBJS): %$(THUMB_SUFFIX).o : %.c $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS)

$(HALLIB): $(COBJS) $(ASMOBJS)
	$(AR) rs $@ $^


.PHONY: all
