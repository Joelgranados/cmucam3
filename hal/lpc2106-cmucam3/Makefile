
all: libs
	$(MAKE) -f hal.mk 
	$(MAKE) -f hal.mk thumb=1

libs:
	$(MAKE) -C ../../lib
	$(MAKE) -C ../../lib thumb=1

clean:
	$(RM) *.a *.o
	$(MAKE) -C ../../lib clean 



.PHONY: clean all libs
