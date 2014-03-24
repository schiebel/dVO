//# dvo_adaptor.cpp:  DBus adaptor for the dVO DBus VO Client
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
#include <iostream>
#include <string>
#include <numeric>
#include <functional>
#include <string.h>
#include <curl/curl.h>
#include <libxml/xmlmemory.h>
#include <dvo/adaptor.hpp>
#include <dvo/dal.hpp>
#include <dvo/libxml2.hpp>

using std::cout;
using std::endl;
using std::get;

//
// See <dvo/libxml2.hpp>...
constexpr unsigned long dvo::xml2::sax::meta[];

namespace dvo {

     typedef size_t (curlfunc_t)(void *ptr,size_t,size_t);

     namespace pvt {
          size_t curlfunc(void *ptr, size_t size, size_t nmemb, void *func) {
               return (*static_cast<std::function<curlfunc_t>*>(func))(ptr,size,nmemb);
          }

          /** SAX2 callback when an element start has been detected by the parser. It provides the namespace informations for the element, as well as the new namespace declarations on the element.
              ctx:  the user data (XML parser context)
              localname:  the local name of the element
              prefix:  the element namespace prefix if available
              URI:  the element namespace name if available
              nb_namespaces: number of namespace definitions on that node
              namespaces: pointer to the array of prefix/URI pairs namespace definitions
              nb_attributes: the number of attributes on that node
              nb_defaulted:  the number of defaulted attributes. The defaulted ones are at the end of the array
              attributes: pointer to the array of (localname/prefix/URI/value/end) attribute values.
          */
          static void startElementNs( const xmlChar * localname, 
                                      const xmlChar * prefix, 
                                      const xmlChar * URI, 
                                      int nb_namespaces, 
                                      const xmlChar ** namespaces, 
                                      int nb_attributes, 
                                      int nb_defaulted, 
                                      const xmlChar ** attributes ) {
               printf( "startElementNs: name = '%s' prefix = '%s' uri = (%p)'%s'\n", localname, prefix, URI, URI );
               for ( int indexNamespace = 0; indexNamespace < nb_namespaces; ++indexNamespace )
               {
                    const xmlChar *prefix = namespaces[indexNamespace*2];
                    const xmlChar *nsURI = namespaces[indexNamespace*2+1];
                    printf( "  namespace: name='%s' uri=(%p)'%s'\n", prefix, nsURI, nsURI );
               }

               unsigned int index = 0;
               for ( int indexAttribute = 0; 
                     indexAttribute < nb_attributes; 
                     ++indexAttribute, index += 5 )
               {
                    const xmlChar *localname = attributes[index];
                    const xmlChar *prefix = attributes[index+1];
                    const xmlChar *nsURI = attributes[index+2];
                    const xmlChar *valueBegin = attributes[index+3];
                    const xmlChar *valueEnd = attributes[index+4];
                    std::string value( (const char *)valueBegin, (const char *)valueEnd );
                    printf( "  %sattribute: localname='%s', prefix='%s', uri=(%p)'%s', value='%s'\n",
                            indexAttribute >= (nb_attributes - nb_defaulted) ? "defaulted " : "",
                            localname,
                            prefix,
                            nsURI,
                            nsURI,
                            value.c_str() );
               }
          }
     }

    void fetch_url( std::string url ) {
         CURL *curl = curl_easy_init( );
         curl_easy_setopt(curl,CURLOPT_URL,url.c_str( ));
         curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pvt::curlfunc);
         int count = 0;
         xml2::sax sax;

         // sax.table.startElementNs = pvt::startElementNs;

         sax.table.startElementNs = [&]( std::string localname, std::string prefix, std::string uri,
                                         std::vector<std::tuple<std::string,std::string>> namespaces,
                                         std::vector<std::tuple<std::string,std::string,std::string,std::string>> attributes ) {
                                            if ( localname == u8"FIELD" ) {
                                                cout << "\tname = " << localname << " prefix = " << prefix << " uri = " << uri << endl;

                                                for ( auto ns : namespaces )
                                                    cout << "\t\tnamespace: name      = " << get<0>(ns) << "  uri = " << get<1>(ns) << endl;

                                                for ( auto attr : attributes )
                                                    cout << "\t\tattribute: localname = " << get<0>(attr) << " prefix = " << get<1>(attr) <<
                                                            " uri =  " << get<2>(attr) << " value = " << get<3>(attr) << endl;
                                            }
                                    };

         sax.table.endElementNs = []( std::string localname, std::string prefix, std::string uri) \
         { /*std::cout << "endElementNs: name = '" << localname << "' prefix = '" << prefix << "' uri = '" << uri << "'" << endl;*/ };
         sax.table.warning = [](std::string msg) { std::cout << "warning: " << msg << std::endl; };
         sax.table.error = [](std::string msg) { std::cout << "error: " << msg << std::endl; };

         auto ctxt = xmlCreatePushParserCtxt( sax.handler( ), &sax, nullptr, 0, nullptr);
         auto cb = std::function<curlfunc_t>(
              [&](void *ptr, size_t size, size_t nmemb) -> size_t {
                   ++count;
                   char buf[35];
                   strncpy(buf,(char*)ptr,30);
                   printf("(%zu/%zu): %s\n",size,nmemb,buf);
                   //printf("%*.*s", size * nmemb, size * nmemb, ptr);
                   xmlParseChunk(ctxt, (const char*) ptr, size*nmemb, 0);
                   return size*nmemb;
              });
         curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cb);
         curl_easy_perform(curl);
         xmlFreeParserCtxt(ctxt);
         curl_easy_cleanup(curl);
         cout << ">>>===========>> " << count << endl;
    }

    adaptor::adaptor( std::string bus_name, std::string object_path ) : casa::dbus::address(bus_name),
                                                                        DBus::ObjectAdaptor( casa::DBusSession::instance( ).connection( ), object_path ),
                                                                        standard_vos{"http://vaosa-vm1.aoc.nrao.edu/ivoa-dal/siapv2"} { }

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
    int32_t adaptor::query( const double& ra, const double& dec,
                            const double& ra_size, const double& dec_size,
                            const std::string& format, const bool& poll,
                            const std::map< std::string, ::DBus::Variant >& params,
                            const std::vector< std::string >& vos) {
        auto qry = dal::Query( standard_vos[0], ra, dec, ra_size, dec_size );

        std::accumulate( params.begin( ), params.end( ), qry,
                         []( dal::Query q, std::pair<std::string,::DBus::Variant> p ) -> dal::Query {
                              if ( p.second.signature( ) == ::DBus::type<uint8_t>::sig( ) ) return q.add(p.first,(long) static_cast<uint8_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<int16_t>::sig( ) ) return q.add(p.first,(long) static_cast<int16_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<uint16_t>::sig( ) ) return q.add(p.first,(long) static_cast<uint16_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<int32_t>::sig( ) ) return q.add(p.first,(long) static_cast<int32_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<uint32_t>::sig( ) ) return q.add(p.first,(long) static_cast<uint32_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<double>::sig( ) ) return q.add(p.first,static_cast<double>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<std::string>::sig( ) ) return q.add(p.first,p.second.operator std::string( ));
                              else throw std::runtime_error(std::string("type conversion failed for: ") + p.first);
                         } );

        cout << "url:\t" << qry.url() << endl;
        cout << "-----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----" << endl;
        fetch_url( qry.url( ) );
        cout << "-----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----" << endl;
         
        return 2003;
    }
}
