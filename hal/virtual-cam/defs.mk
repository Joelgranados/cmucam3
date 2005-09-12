CC=gcc
LD=ld
AR=ar
OBJCOPY=objcopy

ifeq ($(strip $(HALDIR)),)
  HALDIR=.
endif

LIBS=
CFLAGS=-I$(HALDIR) -I$(HALDIR)/../common -O2 -Wextra -Wall -std=gnu99 -g -fdata-sections -ffunction-sections
LDFLAGS=-Wl,--gc-sections

HALLIB=libhal-virtual-cam.a
