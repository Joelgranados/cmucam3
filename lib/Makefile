SUBDIRS=cc3_ilp jpeg-6b

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
