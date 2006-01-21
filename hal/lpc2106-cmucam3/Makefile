include defs.mk

OBJS=interrupt.o cc3.o cc3_hal.o LPC2100.o syscalls.o startup.o serial.o servo.o \
	rdcf2.o mmc_init.o mmc_hardware.o mmc_module.o


all: $(HALLIB)

startup.o: startup.s
	$(CC) -c -o $@ $< $(CFLAGS)

$(HALLIB): $(OBJS)
	$(AR) rs $@ $^


clean:
	$(RM) *.a *.o



.PHONY: clean all
