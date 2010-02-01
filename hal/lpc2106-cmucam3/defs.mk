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


export hal=lpc2106-cmucam3
#HALNAME=$(hal)$(THUMB_SUFFIX)
HALNAME=$(hal)

COMPILER_PREFIX=arm-none-eabi-

CC=$(CCACHE) $(COMPILER_PREFIX)gcc
LD=$(COMPILER_PREFIX)ld
AR=$(COMPILER_PREFIX)ar
OBJCOPY=$(COMPILER_PREFIX)objcopy
SIZE=$(COMPILER_PREFIX)size

ifeq ($(strip $(HALDIR)),)
  HALDIR=.
endif


#DASH_THUMB=-thumb
#ifeq ($(strip $(thumb)),1)
#  THUMB_FLAGS=-mthumb
#  THUMB_SUFFIX=$(DASH_THUMB)
#endif


# set iprintf and iscanf as default (can be overriden by INTEGER_STDIO=0)
INTEGER_STDIO := 1
ifeq ($(strip $(INTEGER_STDIO)),1)
  INTEGER_STDIO_FLAGS := \
	-Dprintf=iprintf \
	$(foreach pp,as f s sn vas vsn vf vs v,-D$(pp)printf=$(pp)iprintf) \
	-Dscanf=iscanf \
	$(foreach pp,f s vf v vs,-D$(pp)scanf=$(pp)iscanf)
endif


LIBS+=
override CFLAGS+=-I$(HALDIR)/../../include -O2 -pipe -funit-at-a-time \
	-Wall -Wstrict-prototypes -Wcast-align -Wcast-qual \
	-Wimplicit -Wmissing-declarations -Wmissing-prototypes \
	-Wnested-externs -Wpointer-arith -Wswitch -Wno-redundant-decls \
	-Wreturn-type -Wshadow -Wstrict-prototypes -Wunused -Wextra \
	-Werror-implicit-function-declaration \
	-ffreestanding -std=gnu99 -g -fdata-sections -ffunction-sections \
	-mcpu=arm7tdmi-s -fno-exceptions -fno-common \
	-msoft-float -mthumb-interwork $(INTEGER_STDIO_FLAGS)

override LDFLAGS+=-lm -T$(HALDIR)/lpc2106-rom.ln \
	-mcpu=arm7tdmi-s -msoft-float

HALLIB=libhal-$(HALNAME).a
