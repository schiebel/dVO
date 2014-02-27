
SUBDIRS := dep src
TARGETS := all clean distclean

$(TARGETS)::
	@for dir in $(SUBDIRS); do   \
	    echo doing $@ in $$dir;  \
	    $(MAKE) -C $$dir $@;     \
	done

clean::
	rm -f mk/*~
	rm -f *~

distclean:: clean
	rm -f config.status configure aclocal.m4 config.log
	rm -f mk/rules.mk
	rm -rf autom4te.cache
	rm -f include/dvo/config.h

.PHONY: $(TARGETS)
