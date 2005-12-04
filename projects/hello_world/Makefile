# set hal if not given by "hal=____"

ifeq ($(strip $(hal)),)
  hal=lpc2106-cmucam3
endif

HALDIR=../../hal/$(hal)

# include definitions (compiler, options, etc)
include $(HALDIR)/defs.mk


OBJS=main.o 

# targets

all: hello_world.hex

hello_world.hex: hello_world 
	$(OBJCOPY) -O ihex $< $@

hello_world: $(OBJS) $(HALDIR)/$(HALLIB)
	$(CC) -o $@ $(OBJS) -L$(HALDIR) \
	-Wl,-whole-archive -lhal-$(hal) -Wl,-no-whole-archive $(LDFLAGS)

clean:
	$(RM) *.elf *.a *.o hello_world *.hex



.PHONY: clean all
