# set hal if not given by "hal=____"

ifeq ($(strip $(hal)),)
  hal=lpc2106-cmucam3
endif

HALDIR=../../hal/$(hal)

# include definitions (compiler, options, etc)
include $(HALDIR)/defs.mk


OBJDIR=$(HALNAME)_buildfiles
OBJS=$(patsubst %.c, $(OBJDIR)/%.o,$(CSOURCES))

# decide if we are building a project or a lib
ifneq ($(strip $(PROJECT)),)
 item=$(PROJECT)_$(HALNAME).hex
else
 PROJECT=$(LIB)
 item=lib$(PROJECT)_$(HALNAME).a
endif


# targets
all: $(item)

$(PROJECT)_$(HALNAME).hex: $(PROJECT)_$(HALNAME)
	@echo "  OBJCOPY $@"
	@$(OBJCOPY) -O ihex $< $@
	@$(SIZE) $<

LIBFILES=$(foreach lib,$(LIBS),../../lib/$(lib)/lib$(lib)_$(HALNAME).a)
LIBDIRS=$(foreach lib,$(LIBS),../../lib/$(lib))
LIBARGS=$(foreach lib,$(LIBS),-l$(lib)_$(HALNAME))

$(PROJECT)_$(HALNAME): $(OBJS) $(HALDIR)/$(HALLIB) $(LIBFILES)
	@echo "  CC      $@"
	@$(CC) -o $@ $(OBJS) -L$(HALDIR) \
	$(foreach ldir,$(LIBDIRS),-L$(ldir)) \
	-Wl,-whole-archive -lhal-$(HALNAME) \
	$(LIBARGS) \
	-Wl,-no-whole-archive $(LDFLAGS)

$(OBJS): $(OBJDIR)/%.o : %.c $(INCLUDES)
	@if [ ! -d $(OBJDIR) ]; then $(RM) $(OBJDIR); \
                                     echo "  MKDIR   $(OBJDIR)"; \
                                     mkdir $(OBJDIR); fi
	@echo "  CC      $@"
	@$(CC) $(CFLAGS) $(foreach ldir,$(LIBDIRS),-I$(ldir)) -o $@ -c $<


# if LIB = something
lib$(PROJECT)_$(HALNAME).a: $(OBJS)
	@echo "  AR      $@"
	@$(AR) rs $@ $^



clean:
	$(RM) *.hex
	$(RM) $(PROJECT)_$(HALNAME)
	$(RM) $(PROJECT)_$(HALNAME).exe
	$(RM) lib$(PROJECT)_$(HALNAME).a
	$(RM) -r $(OBJDIR)



.PHONY: clean all
