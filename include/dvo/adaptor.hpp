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

namespace dvo {

    class adaptor
         : private casa::dbus::address,
           private edu::nrao::casa::dVO_adaptor,
           public DBus::IntrospectableAdaptor,
           public DBus::ObjectAdaptor {
    public:
        adaptor( std::string bus_name, std::string object_path );
        ~adaptor( );
        virtual int32_t fetch(const std::vector< std::string >& urls, const bool& poll);
        virtual std::vector< ::DBus::Variant > request(const int32_t& id);
        virtual int32_t query( const double& ra, const double& dec, const double& ra_size, const double& dec_size,
                               const std::string& format, const bool& poll,
                               const std::map< std::string, ::DBus::Variant >& params,
                               const std::vector< std::string >& vos );
    private:
        // standard list of sites to query
        std::vector<std::string> standard_vos;
    };

}
