# set hal if not given by "hal=____"

ifeq ($(strip $(hal)),)
  hal=lpc2106-cmucam3
endif

HALDIR=../../hal/$(hal)

# include definitions (compiler, options, etc)
include $(HALDIR)/defs.mk


OBJDIR=build-$(HALNAME)
OBJS=$(patsubst %.c, $(OBJDIR)/%.o,$(CSOURCES))

# targets

all: $(HALNAME)-$(PROJECT).hex

$(OBJDIR):
	mkdir $@

$(HALNAME)-$(PROJECT).hex: $(HALNAME)-$(PROJECT)
	$(OBJCOPY) -O ihex $< $@

$(HALNAME)-$(PROJECT): $(OBJS) $(HALDIR)/$(HALLIB)
	$(CC) -o $@ $(OBJS) -L$(HALDIR) \
	-Wl,-whole-archive -lhal-$(hal) -Wl,-no-whole-archive $(LDFLAGS)

$(OBJS): $(OBJDIR)/%.o : %.c $(INCLUDES) $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) *.hex
	$(RM) $(HALNAME)-$(PROJECT)
	$(RM) $(HALNAME)-$(PROJECT).exe
	$(RM) -r $(OBJDIR)



.PHONY: clean all
