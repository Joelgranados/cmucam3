#COMPILER_PREFIX=arm-none-eabi
COMPILER_PREFIX=arm-elf

CC=$(COMPILER_PREFIX)-gcc
LD=$(COMPILER_PREFIX)-ld
AR=$(COMPILER_PREFIX)-ar
OBJCOPY=$(COMPILER_PREFIX)-objcopy

ifeq ($(strip $(HALDIR)),)
  HALDIR=.
endif

LIBS=
CFLAGS=-I$(HALDIR) -I$(HALDIR)/../common -O2 -mcpu=arm7tdmi -Wextra -Wall -ffreestanding -std=gnu99 -g -fdata-sections -ffunction-sections -Werror-implicit-function-declaration -msoft-float -mthumb -mthumb-interwork
LDFLAGS=-nostartfiles -lm -T$(HALDIR)/arm.ln -Wl,--gc-sections -mcpu=arm7tdmi -msoft-float -mthumb -mthumb-interwork


HALLIB=libhal-lpc2106-cmucam3.a
