include ../mk/rules.mk

DBUS_CPP_OBJECTS :=  object.o connection.o dispatcher.o pendingcall.o interface.o message.o \
                     eventloop.o error.o debug.o types.o introspection.o

DBUS_CASA_OBJECTS :=  BusAccess.o DBusSession.o Dispatcher.o

OBJECTS := $(addprefix dbcpp_, $(DBUS_CPP_OBJECTS)) \
           $(addprefix dbcasa_, $(DBUS_CASA_OBJECTS))

all: dVO

dvo_adaptor.o: $(INCPATH)/dvo/dVO.adaptor.hpp $(INCPATH)/dvo/adaptor.hpp
dvo_main.o: $(INCPATH)/dvo/dVO.adaptor.hpp $(INCPATH)/dvo/adaptor.hpp

dVO: dvo_main.o dvo_adaptor.o $(OBJECTS)
	$(CXX) -o $@ $(LDFLAGS) $^ $(LIBS)

clean:
	rm -f $(INCPATH)/dvo/dVO.adaptor.hpp  $(INCPATH)/dvo/dVO.proxy.hpp
	rm -f $(INCPATH)/dvo/*~
	rm -f $(OBJECTS)
	rm -f dvo_main.o dvo_adaptor.o dVO
	rm -f *~

distclean: clean


.PHONY: all clean distclean
