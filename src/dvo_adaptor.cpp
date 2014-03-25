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
#include <deque>
#include <string>
#include <numeric>
#include <iostream>
#include <string.h>
#include <functional>
#include <curl/curl.h>
#include <dvo/dal.hpp>
#include <dvo/adaptor.hpp>
#include <libxml/xmlmemory.h>
#include <dvo/libxml2.hpp>

using std::vector;
using std::string;
using std::tuple;
using std::deque;
using std::cout;
using std::endl;
using std::get;
using std::map;

//
// See <dvo/libxml2.hpp>...
constexpr unsigned long dvo::xml2::sax::meta[];

namespace dvo {

     typedef size_t (curlfunc_t)(void *ptr,size_t,size_t);

     namespace pvt {
          size_t curlfunc(void *ptr, size_t size, size_t nmemb, void *func) {
               return (*static_cast<std::function<curlfunc_t>*>(func))(ptr,size,nmemb);
          }
     }

    void fetch_query( string url ) {
        CURL *curl = curl_easy_init( );
        curl_easy_setopt(curl,CURLOPT_URL,url.c_str( ));
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pvt::curlfunc);
        int count = 0;
        xml2::sax sax;

        string type;
        deque<string> keys_pattern;
        deque<string> keys;           // >>> used for setting up data maps
        map<string,string> values;
        bool collect_characters = false;
        bool inside_table = false;
        string characters;

        sax.table.startElementNs =
            [&]( string name, string prefix, string uri, vector<tuple<string,string>> namespaces,
                 vector<tuple<string,string,string,string>> attributes ) {
                    if ( name == u8"TR" ) { if ( inside_table ) {type = "data"; keys = keys_pattern; values.clear(); } }
                    else if ( name == u8"TD" ) {  if ( inside_table ) { characters = ""; collect_characters = true; } }
                    else if ( name == u8"RESOURCE" ) { type = "begin"; values.clear(); }
                    else if ( name == u8"INFO" && attributes.size( ) > 1 ) {
                        if ( get<0>(attributes[0]) == u8"name" && get<0>(attributes[1]) == u8"value" )
                            values[get<3>(attributes[0])] = get<3>(attributes[1]);
                        else if ( get<0>(attributes[0]) == u8"value" && get<0>(attributes[1]) == u8"name" )
                            values[get<3>(attributes[1])] = get<3>(attributes[0]);
                    }
                    else if ( name == u8"TABLE" ) {
                        /*push out "begin" observable*/
                        type = "description";
                        keys_pattern.clear( );
                        values.clear( );
                    }
                    else if ( name == u8"FIELD" ) {
                        string key, typ, units, size, desc;
                        for ( auto attr : attributes ) {
                            if ( get<0>(attr) == u8"name" ) key = get<3>(attr);
                            else if ( get<0>(attr) == u8"datatype" ) typ = get<3>(attr);
                            else if ( get<0>(attr) == u8"unit" ) units = get<3>(attr);
                            else if ( get<0>(attr) == u8"utype" ) desc = get<3>(attr);
                            else if ( get<0>(attr) == u8"arraysize" && get<3>(attr) != u8"*" ) size = get<3>(attr);
                        }
                        keys_pattern.push_back(key);
                        values[key] = typ + "@" + units + "@" + size + "@" + desc;
                    }
                    else if ( name == u8"DATA" ) {
                         /*push out "description" observable*/
                    }
                    else if ( name == u8"TABLEDATA" ) { inside_table = true; }
        };
                    

        sax.table.endElementNs = 
            [&]( string name, string prefix, string uri) {
                    if ( name == u8"TABLEDATA" ) { inside_table = false; }
                    else if ( name == u8"RESOURCE" ) type = "end";
                    else if ( name == u8"TD" ) {
                        if ( inside_table ) {
                            values[keys.front( )] = characters;
                            keys.pop_front( );
                            collect_characters = false;
                        }
                    } else if ( name == u8"TR" ) { if ( inside_table ) { /* shift "data" onto observables */ } }
            };

        sax.table.characters = [&]( string chars ) { if ( collect_characters ) characters += chars; };

        sax.table.warning = [](string msg) { std::cout << "warning: " << msg << std::endl; };
        sax.table.error = [](string msg) { std::cout << "error: " << msg << std::endl; };

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

    adaptor::adaptor( string bus_name, string object_path ) : casa::dbus::address(bus_name),
                                                                        DBus::ObjectAdaptor( casa::DBusSession::instance( ).connection( ), object_path ),
                                                                        standard_vos{"http://vaosa-vm1.aoc.nrao.edu/ivoa-dal/siapv2"} { }

    adaptor::~adaptor( ) {
         /* signal: disconnect( ); */
    }

    int32_t adaptor::fetch(const vector< string >& urls, const bool& poll) {
        cout << "adaptor::fetch( [";
        for ( auto v: urls ) cout << v << " ";
        cout << "], " << poll << " )" << endl;
        return 2001;
    }
    vector< ::DBus::Variant > adaptor::request(const int32_t& id) {
        cout << "adaptor::request( " << id << " )" << endl;
        return vector<::DBus::Variant>( );
    }
    int32_t adaptor::query( const double& ra, const double& dec,
                            const double& ra_size, const double& dec_size,
                            const string& format, const bool& poll,
                            const map< string, ::DBus::Variant >& params,
                            const vector< string >& vos) {

        // set up the initial query parameters...
        auto qry = dal::Query( standard_vos[0], ra, dec, ra_size, dec_size );

        // add in the extra query parameters...
        // query server ignores unknown/unexpected parameters...
        std::accumulate( params.begin( ), params.end( ), qry,
                         []( dal::Query q, std::pair<string,::DBus::Variant> p ) -> dal::Query {
                              if ( p.second.signature( ) == ::DBus::type<uint8_t>::sig( ) )
                                  return q.add(p.first,(long) static_cast<uint8_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<int16_t>::sig( ) )
                                  return q.add(p.first,(long) static_cast<int16_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<uint16_t>::sig( ) )
                                  return q.add(p.first,(long) static_cast<uint16_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<int32_t>::sig( ) )
                                  return q.add(p.first,(long) static_cast<int32_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<uint32_t>::sig( ) )
                                  return q.add(p.first,(long) static_cast<uint32_t>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<double>::sig( ) )
                                  return q.add(p.first,static_cast<double>(p.second));
                              else if ( p.second.signature( ) == ::DBus::type<string>::sig( ) )
                                  return q.add(p.first,p.second.operator string( ));
                              else throw std::runtime_error(string("type conversion failed for: ") + p.first);
                         } );

        cout << "url:\t" << qry.url() << endl;
        cout << "-----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----" << endl;
        fetch_query( qry.url( ) );
        cout << "-----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----  -----" << endl;

        return 2003;
    }
}
