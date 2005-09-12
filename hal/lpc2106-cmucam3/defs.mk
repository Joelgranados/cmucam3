CC=arm-elf-gcc
LD=arm-elf-ld
AR=arm-elf-ar
OBJCOPY=arm-elf-objcopy

ifeq ($(strip $(HALDIR)),)
  HALDIR=.
endif

LIBS=
CFLAGS=-I$(HALDIR) -I$(HALDIR)/../common -O2 -mcpu=arm7tdmi -Wextra -Wall -ffreestanding -std=gnu99 -g -fdata-sections -ffunction-sections
LDFLAGS=-nostartfiles -lc -lm -lc -lgcc -T$(HALDIR)/arm.ln -Wl,--gc-sections


HALLIB=libhal-lpc2106-cmucam3.a
