# set hal if not given by "hal=____"

ifeq ($(strip $(hal)),)
  hal=lpc2106-cmucam3
endif

HALDIR=../../hal/$(hal)

# include definitions (compiler, options, etc)
include $(HALDIR)/defs.mk


OBJS=main.o


# targets

all: cmucam2.hex

cmucam2.hex: cmucam2

cmucam2: $(OBJS) $(HALDIR)/$(HALLIB)


clean:
	$(RM) *.elf *.a *.o cmucam2 *.hex



.PHONY: clean all
