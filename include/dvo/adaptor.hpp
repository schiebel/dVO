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
        virtual int32_t query(const std::string& url, const bool& poll);
    };

}
