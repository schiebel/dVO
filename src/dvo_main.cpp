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
#include <dvo/util.hpp>
#include <dbus/dbus.h>
#include <unistd.h>
#include <signal.h>


using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::make_shared;
using namespace dvo;

int main( int argc, char *argv[] ) {

    // make dbus thread aware...
    dbus_threads_init_default( );
    // ensure libxml2 headers/libraries match...
    LIBXML_TEST_VERSION

	auto adaptor = make_shared<dvo::adaptor>("dVO","/casa/dVO");

	for ( int i=0; i < argc; ++i ) {
		std::string arg {argv[i]};
		{	std::string prefix = "--query-file=";
			if ( arg.size( ) > prefix.size( ) && equal( prefix.begin( ), prefix.end( ), arg.begin( ) ) ) {
				string query_file = arg.substr( prefix.size( ) );
				if ( util::exists( query_file ) )
					adaptor->set_query_result(query_file);
				else
					util::die( "could not read query test file: ", query_file );
			}
		}
		{	std::string prefix = "--fetch-file=";
			if ( arg.size( ) > prefix.size( ) && equal( prefix.begin( ), prefix.end( ), arg.begin( ) ) ) {
				string fetch_file = arg.substr( prefix.size( ) );
				if ( util::exists( fetch_file ) )
					adaptor->set_fetch_result(fetch_file);
				else
					util::die( "could not read fetch test file: ", fetch_file );
			}
		}
		{	std::string match = "--testing";
			if ( arg.size( ) == match.size( ) && equal( match.begin( ), match.end( ), arg.begin( ) ) ) {
				if ( adaptor->mode( ) == adaptor::Mode::logging )
					util::die( "cannot use both --testing (reading from test files) and --logging (saving to test files)" );
				adaptor->mode(adaptor::Mode::testing);
			}
		}
		{	std::string match = "--logging";
			if ( arg.size( ) == match.size( ) && equal( match.begin( ), match.end( ), arg.begin( ) ) ) {
				if ( adaptor->mode( ) == adaptor::Mode::testing )
					util::die( "cannot use both --testing (reading from test files) and --logging (saving to test files)" );
				adaptor->mode(adaptor::Mode::logging);
			}
		}
	}

    casa::DBusSession::instance( ).dispatcher( ).enter( );
}
