HALNAME=lpc2106-cmucam3$(THUMB_SUFFIX)

COMPILER_PREFIX=arm-none-eabi
#COMPILER_PREFIX=arm-elf

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
