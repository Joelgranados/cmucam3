# Copyright 2006  Anthony Rowe and Adam Goode
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.



# set hal if not given by "hal=____"
#hal=virtual-cam

ifeq ($(strip $(hal)),)
  hal=lpc2106-cmucam3
endif

INCLUDES += ../../include/cc3.h
HALDIR=../../hal/$(hal)

# include definitions (compiler, options, etc)
include $(HALDIR)/defs.mk

ifneq ($(strip $(V)),1)
  VV=@
endif


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
	$(VV)$(OBJCOPY) -O ihex $< $@
	$(VV)$(SIZE) $<

LIBFILES=$(foreach lib,$(LIBS),../../lib/$(lib)/lib$(lib)_$(HALNAME).a)
LIBDIRS=$(foreach lib,$(LIBS),../../lib/$(lib))
LIBARGS=$(foreach lib,$(LIBS),-l$(lib)_$(HALNAME))

# the whole-archive stuff is needed, because we need to include
# the syscalls -- gc-sections will eliminate useless symbols
$(PROJECT)_$(HALNAME): $(OBJS) $(HALDIR)/$(HALLIB) $(LIBFILES)
	@echo "  CC      $@"
	$(VV)$(CC) -o $@ $(OBJS) -L$(HALDIR) \
	$(foreach ldir,$(LIBDIRS),-L$(ldir)) \
	$(LIBARGS) \
	-Wl,-Map=$(PROJECT)_$(HALNAME).map \
	-Wl,--cref \
	-Wl,--gc-sections \
	-Wl,-whole-archive \
	-lhal-$(HALNAME) \
	-Wl,-no-whole-archive \
	$(LDFLAGS)

$(OBJDIR):
	@echo "  MKDIR   $@"
	mkdir $@

$(OBJS): $(OBJDIR)/%.o : %.c $(INCLUDES) | $(OBJDIR)
	@echo "  CC      $@"
	$(VV)$(CC) $(CFLAGS) $(foreach ldir,$(LIBDIRS),-I$(ldir)) -o $@ -c $<


# if LIB = something
lib$(PROJECT)_$(HALNAME).a: $(OBJS)
	@echo "  AR      $@"
	$(VV)$(AR) rs $@ $^



clean:
	$(RM) *.hex
	$(RM) *.map
	$(RM) $(PROJECT)_$(HALNAME)
	$(RM) $(PROJECT)_$(HALNAME).exe
	$(RM) lib$(PROJECT)_$(HALNAME).a
	$(RM) -r $(OBJDIR)



.PHONY: clean all
