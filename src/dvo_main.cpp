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
#include <unistd.h>
#include <signal.h>

#include <dvo/SysState.hpp>

using std::cout;
using std::endl;
using dvo::SysState;
using dvo::SysStateObservable;

int main( int argc, char *argv[] ) {

    // make dbus thread aware...
    dbus_threads_init_default( );
    // ensure libxml2 headers/libraries match...
    LIBXML_TEST_VERSION

    auto adaptor = std::make_shared<dvo::adaptor>("dVO","/casa/dVO");

	unsigned remaining_seconds;
	auto sleepStream = SysStateObservable( );
	rxcpp::from(sleepStream).
		subscribe( [&](SysState e) {
				switch(e) {
				case SysState::WillSleep:
					remaining_seconds = alarm(0);
					break;
				case SysState::HasPoweredOn:
					alarm(remaining_seconds);
					break;
				} } );
     
    cout << "Hello World..." << endl;
    casa::DBusSession::instance( ).dispatcher( ).enter( );
}
