SUBDIRS=lpc2106-cmucam3 lpc2103-tcm8240 virtual-cam
#SUBDIRS=lpc2106-cmucam3
#SUBDIRS=virtual-cam

# Build rules
all:
	@list='$(SUBDIRS)'; set -e;  for subdir in $$list; do \
	  $(MAKE) -C $$subdir; \
	done

clean:
	@list='$(SUBDIRS)'; set -e;  for subdir in $$list; do \
	  $(MAKE) -C $$subdir clean; \
	done


.PHONY: all clean
