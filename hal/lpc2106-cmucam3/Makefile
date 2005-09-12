include defs.mk

OBJS=interrupt.o cc3.o LPC2100.o syscalls.o startup.o serial.o


all: $(HALLIB)

startup.o: startup.s
	$(CC) -c -o $@ $< $(CFLAGS)

$(HALLIB): $(OBJS)
	$(AR) rs $@ $^


clean:
	$(RM) *.a *.o



.PHONY: clean all
