//# dvo_main.cpp:  main routine for the dVO DBus VO Client
//# Copyright (C) 2014
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#include <memory>
#include <iostream>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <dvo/adaptor.hpp>
#include <dbus/dbus.h>

using std::cout;
using std::endl;
int main( int argc, char *argv[] ) {
     // make dbus thread aware...
     dbus_threads_init_default( );
     // ensure libxml2 headers/libraries match...
     LIBXML_TEST_VERSION

     auto adaptor = std::make_shared<dvo::adaptor>("dVO","/casa/dVO");
     
     cout << "Hello World..." << endl;
     casa::DBusSession::instance( ).dispatcher( ).enter( );
}
