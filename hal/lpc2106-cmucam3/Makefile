include defs.mk

OBJS=interrupt.o cc3.o LPC2100.o serial.o startup.s


all: $(HALLIB)

$(HALLIB): $(OBJS)
	$(AR) rs $@ $^


clean:
	$(RM) *.a *.o



.PHONY: clean all
