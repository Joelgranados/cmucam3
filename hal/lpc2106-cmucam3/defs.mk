CC=arm-elf-gcc
LD=arm-elf-ld
AR=arm-elf-ar
OBJCOPY=arm-elf-objcopy

ifeq ($(strip $(HALDIR)),)
  HALDIR=.
endif

LIBS=
CFLAGS=-I$(HALDIR) -I$(HALDIR)/../common -O2 -mcpu=arm7tdmi -mthumb-interwork -Wextra -Wall -ffreestanding -std=gnu99 -g
LDFLAGS=


HALLIB=libhal-lpc2106-cmucam3.a
