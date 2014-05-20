//# adaptor.hpp: dVO dbus adaptor
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
#pragma once

#include <dvo/dVO.adaptor.hpp>
#include <casadbus/utilities/BusAccess.h>
#include <casadbus/session/DBusSession.h>
#include <dbus-cpp/dbus.h>
#include <rxcpp/rx.hpp>
#include <mutex>

namespace dvo {

    class adaptor
         : private casa::dbus::address,
           private edu::nrao::casa::dVO_adaptor,
           public DBus::IntrospectableAdaptor,
           public DBus::ObjectAdaptor {
    public:
		enum class Mode { normal, testing, logging };

        adaptor( std::string bus_name, std::string object_path );
        ~adaptor( );
		virtual void cancel(const int32_t &id);
        virtual int32_t fetch(const std::string &url, const std::string &output, const bool& progress);
        virtual int32_t query( const double& ra, const double& dec, const double& ra_size, const double& dec_size,
                               const std::string& format,
                               const std::map< std::string, ::DBus::Variant >& params,
                               const std::vector< std::string >& vos );

		// set the results for all queries and fetches to a constant result for testing and debugging...
		void set_query_result( const std::string &query_file ) { query_result_file = query_file; }
		void set_fetch_result( const std::string &fetch_file ) { fetch_result_file = fetch_file; }

		void mode( Mode new_mode ) { mode_ = new_mode; }
		Mode mode( ) const { return mode_; }

    private:
        // standard list of sites to query
        std::vector<std::string> standard_vos;
		int32_t last_id;
		// mode determines the treatment (input or output) for test/debugging files...
		Mode mode_;
		std::string query_result_file;
		std::string fetch_result_file;

		std::mutex                      pending_;
		std::map<int,rxcpp::Disposable> pending;
    };

}
