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


#HALNAME=lpc2106-cmucam3$(THUMB_SUFFIX)
export hal=virtual-cam
HALNAME=$(hal)

#COMPILER_PREFIX=arm-none-eabi-
COMPILER_PREFIX=

CC=$(CCACHE) $(COMPILER_PREFIX)gcc
LD=$(COMPILER_PREFIX)ld
AR=$(COMPILER_PREFIX)ar
OBJCOPY=$(COMPILER_PREFIX)objcopy
SIZE=$(COMPILER_PREFIX)size

ifeq ($(strip $(HALDIR)),)
  HALDIR=.
endif


LIBS+=
override CFLAGS+=-I$(HALDIR)/../../include -Os -pipe -funit-at-a-time \
	-Wall -Wstrict-prototypes -Wcast-align -Wcast-qual \
	-Wimplicit -Wmissing-declarations -Wmissing-prototypes \
	-Wnested-externs -Wpointer-arith -Wswitch -Wno-redundant-decls \
	-Wreturn-type -Wshadow -Wstrict-prototypes -Wunused -Wextra \
	-Werror-implicit-function-declaration \
	-ffreestanding -std=gnu99 -g -fdata-sections -ffunction-sections \
	-DVIRTUAL_CAM

override LDFLAGS+=-lm

HALLIB=libhal-$(HALNAME).a
