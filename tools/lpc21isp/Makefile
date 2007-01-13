CC=gcc
OBJS=lpc21isp.o
CFLAGS=-O2 -Wall
INSTALLDIR=/usr/local/bin/

all: lpc21isp

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

lpc21isp: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f $(OBJS) lpc21isp

install:
	install -m 755 lpc21isp /usr/local/bin/lpc21isp

.PHONY: clean
