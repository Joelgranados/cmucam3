
all: hal libs

hal:
	$(MAKE) -f hal.mk 
	$(MAKE) -f hal.mk thumb=1

libs:
	$(MAKE) -C ../../lib
	$(MAKE) -C ../../lib thumb=1

clean:
	$(RM) *.a *.o
	$(MAKE) -C ../../lib clean 
	$(MAKE) -C ../../lib clean thumb=1



.PHONY: clean all libs hal
