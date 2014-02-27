#include <iostream>
#include <dvo/adaptor.hpp>

using std::cout;
using std::endl;

namespace dvo {

     adaptor::adaptor( std::string bus_name, std::string object_path ) : casa::dbus::address(bus_name),
                                                                         DBus::ObjectAdaptor( casa::DBusSession::instance( ).connection( ), object_path ) { }
     adaptor::~adaptor( ) {
          /* signal: disconnect( ); */
     }

    int32_t adaptor::fetch(const std::vector< std::string >& urls, const bool& poll) {
         cout << "adaptor::fetch( [";
         for ( auto v: urls ) cout << v << " ";
         cout << "], " << poll << " )" << endl;
         return 2001;
    }
    std::vector< ::DBus::Variant > adaptor::request(const int32_t& id) {
         cout << "adaptor::request( " << id << " )" << endl;
         return std::vector<::DBus::Variant>( );
    }
    int32_t adaptor::query(const std::string& url, const bool& poll) {
         cout << "adaptor::query( " << url << ", " << poll << " )" << endl;
         return 2003;
    }
}
