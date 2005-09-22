# set hal if not given by "hal=____"

ifeq ($(strip $(hal)),)
  hal=lpc2106-cmucam3
endif

HALDIR=../../hal/$(hal)

# include definitions (compiler, options, etc)
include $(HALDIR)/defs.mk


OBJS=main.o 

# targets

all: jpeg.hex

jpeg.hex: jpeg 
	$(OBJCOPY) -O ihex $< $@

jpeg: $(OBJS) $(HALDIR)/$(HALLIB)
	$(CC) -o $@ $(OBJS) -L$(HALDIR) \
	-Wl,-whole-archive -lhal-$(hal) -Wl,-no-whole-archive $(LDFLAGS)

clean:
	$(RM) *.elf *.a *.o jpeg *.hex



.PHONY: clean all
