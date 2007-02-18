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

CSOURCES=cc3.c cc3_hal.c serial.c \
	gpio.c servo.c
INCLUDES=cc3_hal.h devices.h lpc_config.h \
	cc3_pin_defines.h LPC2100.h serial.h \
	gpio.h servo.h ../../include/cc3.h

COBJS=$(patsubst %.c, %.o,$(CSOURCES))
ASMOBJS=$(patsubst %.s, %$.o,$(ASMSOURCES))

all: $(HALLIB)

$(ASMOBJS): %.o : %.s $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS)

$(COBJS): %.o : %.c $(INCLUDES)
	$(CC) -c -o $@ $< $(CFLAGS)

$(HALLIB): $(COBJS) $(ASMOBJS)
	$(AR) rs $@ $^


.PHONY: all
