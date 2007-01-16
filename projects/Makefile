SUBDIRS=cmucam2 hello_world jpeg-cam benchmark

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
