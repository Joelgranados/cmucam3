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


include defs.mk

CSOURCES=cc3.c cc3_hal.c serial.c \
	gpio.c servo.c
INCLUDES=cc3_hal.h devices.h lpc_config.h \
	cc3_pin_defines.h LPC2100.h serial.h \
	gpio.h servo.h

COBJS=$(patsubst %.c, %$(THUMB_SUFFIX).o,$(CSOURCES))
ASMOBJS=$(patsubst %.s, %$(THUMB_SUFFIX).o,$(ASMSOURCES))

all: $(HALLIB)

$(ASMOBJS): %$(THUMB_SUFFIX).o : %.s $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS)

$(COBJS): %$(THUMB_SUFFIX).o : %.c $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS)

$(HALLIB): $(COBJS) $(ASMOBJS)
	$(AR) rs $@ $^


.PHONY: all
