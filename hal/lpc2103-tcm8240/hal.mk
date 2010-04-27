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


include defs.mk

CSOURCES=interrupt.c cc3.c cc3_hal.c syscalls.c serial.c \
	servo.c rdcf2.c mmc_hardware.c mmc_driver.c gpio.c \
	devices.c uart_driver.c
CSOURCES_FIQ=fast-interrupt.c

INCLUDES=cc3_hal.h devices.h lpc_config.h rdcf2.h servo.h \
	cc3_pin_defines.h interrupt.h LPC2100.h mmc_hardware.h serial.h spi.h \
	gpio.h ../../include/cc3.h
ASMSOURCES=startup.s


#COBJS=$(patsubst %.c, %$(THUMB_SUFFIX).o,$(CSOURCES))
#ASMOBJS=$(patsubst %.s, %$(THUMB_SUFFIX).o,$(ASMSOURCES))
COBJS=$(patsubst %.c, %.o,$(CSOURCES))
COBJS_FIQ=$(patsubst %.c, %.o,$(CSOURCES_FIQ))
ASMOBJS=$(patsubst %.s, %.o,$(ASMSOURCES))

all: $(HALLIB)

#$(ASMOBJS): %$(THUMB_SUFFIX).o : %.s $(INCLUDES)
$(ASMOBJS): %.o : %.s $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS)

#$(COBJS): %$(THUMB_SUFFIX).o : %.c $(INCLUDES)
$(COBJS): %.o : %.c $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS)

$(COBJS_FIQ): %.o : %.c $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS) -ffixed-r0 -ffixed-r1 -ffixed-r2 -ffixed-r3 -ffixed-r4 -ffixed-r5 -ffixed-r6 -ffixed-r7

$(HALLIB): $(COBJS) $(COBJS_FIQ) $(ASMOBJS)
	$(AR) rs $@ $^


.PHONY: all
