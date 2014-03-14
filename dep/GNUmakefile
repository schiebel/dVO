include ../mk/rules.mk

$(INCPATH)/dbus-cpp/%.h: dbus-cpp/include/dbus-cpp/%.h
	$(MKDIR) $(dir $@)
	@echo $<
	$(CP) $< $@

$(INCPATH)/casadbus/types/%.h: casadbus/types/%.h
	$(MKDIR) $(dir $@)
	@echo $<
	$(CP) $< $@

$(INCPATH)/casadbus/utilities/%.h: casadbus/utilities/%.h
	$(MKDIR) $(dir $@)
	@echo $<
	$(CP) $< $@

$(INCPATH)/casadbus/session/%.h: casadbus/session/%.h
	$(MKDIR) $(dir $@)
	@echo $<
	$(CP) $< $@

$(INCPATH)/casadbus/session/%.proxy.h: casadbus/xml/%.interface.xml
	@$(MKDIR) $(dir $@)
	@echo $<
	$(DBUSXML2CPP) $< --proxy=$@

$(INCPATH)/voc/%.h: voclient/libdalclient/%.h
	$(MKDIR) $(dir $@)
	@echo $<
	$(CP) $< $@

### casadbus header files that need to be exported...
CDB_TYPES_HEADERS := variant.h
CDB_UTIL_HEADERS := BusAccess.h
CDB_SESSION_HEADERS := DBusSession.h DBusSession.proxy.h Dispatcher.h
### dbus-cpp header files that need to be exported...
DBCPP_HEADERS := dbus.h types.h api.h message.h error.h interface.h object.h connection.h pendingcall.h \
                 util.h debug.h property.h server.h dispatcher.h eventloop.h eventloop-integration.h \
                 introspection.h pipe.h refptr_impl.h
### VOClient header files that need to be exported...
VOC_HEADERS := VOClient.h

DBUS_CASA_HEADERS = $(addprefix $(INCPATH)/casadbus/types/, $(CDB_TYPES_HEADERS)) \
                   $(addprefix $(INCPATH)/casadbus/utilities/, $(CDB_UTIL_HEADERS)) \
                   $(addprefix $(INCPATH)/casadbus/session/, $(CDB_SESSION_HEADERS))

DBUS_CPP_HEADERS = $(addprefix $(INCPATH)/dbus-cpp/, $(DBCPP_HEADERS))

VOCLIENT_HEADERS = $(addprefix $(INCPATH)/voc/, $(VOC_HEADERS))

###
### first time through, files get extracted and checked out, but
### gmake cannot deduce that "includes" dependencies are satisfied
### unless make is rerun (perhaps there's another way?)...
###
all: voclient casadbus dbus-cpp 
	@$(MAKE) includes

includes: $(DBUS_CASA_HEADERS) $(DBUS_CPP_HEADERS) $(VOCLIENT_HEADERS)

### tar preserves the creation timestamp
voclient: voclient.tgz
	$(TAR) zxf $<
	if test -e $@; then touch $@; fi

casadbus:
	svn co https://svn.cv.nrao.edu/svn/casa/trunk/code/casadbus

dbus-cpp:
	svn co https://svn.cv.nrao.edu/svn/casa-3rd-party/trunk/dbus-cpp

clean:
	rm -f $(DBUS_CASA_HEADERS) $(DBUS_CPP_HEADERS)
	@if test -d $(INCPATH)/dbus-cpp; then rmdir $(INCPATH)/dbus-cpp; fi
	@if test -d	$(INCPATH)/casadbus/session; then rmdir $(INCPATH)/casadbus/session; fi
	@if test -d	$(INCPATH)/casadbus/types; then rmdir $(INCPATH)/casadbus/types; fi
	@if test -d	$(INCPATH)/casadbus/utilities; then rmdir $(INCPATH)/casadbus/utilities; fi
	@if test -d	$(INCPATH)/casadbus; then rmdir $(INCPATH)/casadbus; fi
	rm -f *~

distclean: clean
	rm -rf voclient
	rm -rf dbus-cpp
	rm -rf casadbus


.PHONY: clean distclean
