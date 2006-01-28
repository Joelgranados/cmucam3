# set hal if not given by "hal=____"

ifeq ($(strip $(hal)),)
  hal=lpc2106-cmucam3
endif

HALDIR=../../hal/$(hal)

# include definitions (compiler, options, etc)
include $(HALDIR)/defs.mk


OBJDIR=$(HALNAME)_buildfiles
OBJS=$(patsubst %.c, $(OBJDIR)/%.o,$(CSOURCES))

# targets

all: $(PROJECT)_$(HALNAME).hex

$(OBJDIR):
	mkdir $@

$(PROJECT)_$(HALNAME).hex: $(PROJECT)_$(HALNAME)
	$(OBJCOPY) -O ihex $< $@

$(PROJECT)_$(HALNAME): $(OBJS) $(HALDIR)/$(HALLIB)
	$(CC) -o $@ $(OBJS) -L$(HALDIR) \
	-Wl,-whole-archive -lhal-$(HALNAME) -Wl,-no-whole-archive $(LDFLAGS)

$(OBJS): $(OBJDIR)/%.o : %.c $(INCLUDES) $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) *.hex
	$(RM) $(PROJECT)_$(HALNAME)
	$(RM) $(PROJECT)_$(HALNAME).exe
	$(RM) -r $(OBJDIR)



.PHONY: clean all
