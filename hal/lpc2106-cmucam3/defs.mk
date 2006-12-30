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


HALNAME=lpc2106-cmucam3$(THUMB_SUFFIX)
#HALNAME=virtual_cam

COMPILER_PREFIX=arm-none-eabi
#COMPILER_PREFIX=

CC=$(CCACHE) $(COMPILER_PREFIX)-gcc
LD=$(COMPILER_PREFIX)-ld
AR=$(COMPILER_PREFIX)-ar
OBJCOPY=$(COMPILER_PREFIX)-objcopy
SIZE=$(COMPILER_PREFIX)-size

ifeq ($(strip $(HALDIR)),)
  HALDIR=.
endif


DASH_THUMB=-thumb
ifeq ($(strip $(thumb)),1)
  THUMB_FLAGS=-mthumb
  THUMB_SUFFIX=$(DASH_THUMB)
endif

LIBS+=
override CFLAGS+=-I$(HALDIR)/../../include -Os -funit-at-a-time -mcpu=arm7tdmi-s -Wall -Wstrict-prototypes -Wcast-align -Wcast-qual -Wimplicit -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wswitch -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wunused -Wextra -Werror-implicit-function-declaration -ffreestanding -std=gnu99 -g -fdata-sections -ffunction-sections -msoft-float -mthumb-interwork $(THUMB_FLAGS)
override LDFLAGS+=-nostartfiles -lm -T$(HALDIR)/lpc2106-rom.ln -mcpu=arm7tdmi-s -msoft-float $(THUMB_FLAGS)

HALLIB=libhal-$(HALNAME).a
