# Copyright 2006  Anthony Rowe and Adam Goode
#
# This file is part of cc3.
#
# cc3 is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# cc3 is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cc3; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


# set hal if not given by "hal=____"
#hal=virtual-cam

ifeq ($(strip $(hal)),)
  hal=lpc2106-cmucam3
endif

INCLUDES += ../../include/cc3.h
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

# the whole-archive stuff is needed, because we need to include
# the syscalls -- gc-sections will eliminate useless symbols
$(PROJECT)_$(HALNAME): $(OBJS) $(HALDIR)/$(HALLIB) $(LIBFILES)
	@echo "  CC      $@"
	@$(CC) -o $@ $(OBJS) -L$(HALDIR) \
	$(foreach ldir,$(LIBDIRS),-L$(ldir)) \
	$(LIBARGS) \
	-Wl,-Map=$(PROJECT)_$(HALNAME).map \
	-Wl,--cref \
	-Wl,--gc-sections \
	-Wl,-whole-archive \
	-lhal-$(HALNAME) \
	-Wl,-no-whole-archive \
	$(LDFLAGS)

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
	$(RM) *.map
	$(RM) $(PROJECT)_$(HALNAME)
	$(RM) $(PROJECT)_$(HALNAME).exe
	$(RM) lib$(PROJECT)_$(HALNAME).a
	$(RM) -r $(OBJDIR)



.PHONY: clean all
