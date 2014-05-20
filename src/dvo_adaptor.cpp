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
#include <string.h>
#include <functional>
#include <curl/curl.h>
#include <dvo/dal.hpp>
#include <dvo/util.hpp>
#include <dvo/adaptor.hpp>
#include <libxml/xmlmemory.h>
#include <dvo/libxml2.hpp>
#include <iostream>
#include <fstream>

using std::shared_ptr;
using std::ifstream;
using std::function;
using std::vector;
using std::string;
using std::tuple;
using std::deque;
using std::cout;
using std::endl;
using std::get;
using std::map;

using OBS = tuple<string,int,string,map<string,string>>;
using PROGRESS = tuple<string,int,string,string,double,double,double,double>;

//
// See <dvo/libxml2.hpp>...
constexpr unsigned long dvo::xml2::sax::meta[];

namespace dvo {

	namespace curl {
		using write_t = size_t (void *ptr,size_t,size_t);
		using progress_t = int ( double download_total, double download_total_size_done, double ultotal, double uldone );
	}

    namespace pvt {
        size_t curlwrite(void *ptr, size_t size, size_t nmemb, void *func) {
            return (*static_cast<function<curl::write_t>*>(func))(ptr,size,nmemb);
        }

		int curlprogress(void *func, double download_total_size, double download_total_size_done, double ultotal, double uldone) {
            return (*static_cast<function<curl::progress_t>*>(func))( download_total_size, download_total_size_done, ultotal, uldone );
        }

    }

// query    ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ ------
static shared_ptr<rxcpp::Observable<OBS>> create_subject( int id, function<void(int, shared_ptr<rxcpp::Observer<OBS>>, string, string, rxcpp::Scheduler::shared )> func, string vo_service, string url, rxcpp::Scheduler::shared scheduler = nullptr ) {
    if ( ! scheduler ) {
        scheduler = std::make_shared<rxcpp::EventLoopScheduler>( );
    }
    auto subject = rxcpp::CreateSubject<OBS>( );
    scheduler->Schedule( rxcpp::fix0([=](rxcpp::Scheduler::shared s, function<rxcpp::Disposable(rxcpp::Scheduler::shared)> self) -> rxcpp::Disposable {
                                 func( id, subject, vo_service, url, scheduler );
                                 return rxcpp::Disposable::Empty( );
                         }) );
    return subject;
}
// fetch    ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ ------ ------
	static shared_ptr<rxcpp::Observable<PROGRESS>> create_subject( int id, function<void(int, shared_ptr<rxcpp::Observer<PROGRESS>>, string, string, bool, rxcpp::Scheduler::shared )> func, string url, string output, bool progress, rxcpp::Scheduler::shared scheduler = nullptr ) {
    if ( ! scheduler ) {
        scheduler = std::make_shared<rxcpp::EventLoopScheduler>( );
    }
    auto subject = rxcpp::CreateSubject<PROGRESS>( );
    scheduler->Schedule( rxcpp::fix0([=](rxcpp::Scheduler::shared s, function<rxcpp::Disposable(rxcpp::Scheduler::shared)> self) -> rxcpp::Disposable {
                                 func( id, subject, url, output, progress, scheduler );
                                 return rxcpp::Disposable::Empty( );
                         }) );
    return subject;
}

void process_fetch( int id, shared_ptr<rxcpp::Observer<PROGRESS>> obs, string url, string destination, bool post_progress, rxcpp::Scheduler::shared scheduler ) {
	auto progress = function<curl::progress_t>(
				   [&]( double download_total_size, double download_total_size_done, double ultotal, double uldone ) -> int {
						if ( post_progress || true ) {
							obs->OnNext(PROGRESS( "progress", id, destination, "", download_total_size, download_total_size_done, ultotal, uldone ));
						}
						return 0;
				   });

    FILE *outfile = fopen( destination.c_str(), "w");
	if ( ! outfile ) {
		obs->OnNext(PROGRESS( "error", id, destination, "could not write to output location", 0, 0, 0, 0 ));
		static auto excp = std::make_exception_ptr(std::runtime_error("could not write to output location"));
		obs->OnError(excp);
	}

    char error[CURL_ERROR_SIZE];
    memset(error, 0, CURL_ERROR_SIZE);

	CURL *curlp = curl_easy_init( );

	curl_easy_setopt(curlp, CURLOPT_URL,               url.c_str());
    curl_easy_setopt(curlp, CURLOPT_ERRORBUFFER,       error);

	// progess stuff
	curl_easy_setopt(curlp, CURLOPT_NOPROGRESS,        0);
	curl_easy_setopt(curlp, CURLOPT_PROGRESSDATA,      &progress);
	curl_easy_setopt(curlp, CURLOPT_PROGRESSFUNCTION,  pvt::curlprogress);
	curl_easy_setopt(curlp, CURLOPT_WRITEDATA,         outfile);

	// http related settings
	curl_easy_setopt(curlp, CURLOPT_FOLLOWLOCATION,   1); // follow redirects
	curl_easy_setopt(curlp, CURLOPT_AUTOREFERER,      1); // set the Referer: field in requests where it follows a Location: redirect.
	curl_easy_setopt(curlp, CURLOPT_MAXREDIRS,        20);
	curl_easy_setopt(curlp, CURLOPT_USERAGENT,        "DBus_VO_Service/1.0");
	curl_easy_setopt(curlp, CURLOPT_FILETIME,         1);

	CURLcode status = curl_easy_perform(curlp);
	switch( status ) {
	case CURLE_OK:
		obs->OnNext(PROGRESS( "complete", id, destination, "", 0, 0, 0, 0 ));
		obs->OnCompleted( );
		break;
	default:
		{	// later we could expand this out to all options, see:
			// http://curl.haxx.se/libcurl/c/libcurl-errors.html
			obs->OnNext(PROGRESS( "error", id, destination, error, 0, 0, 0, 0 ));
			static auto excp = std::make_exception_ptr(std::runtime_error(error));
			obs->OnError(excp);
		}
		break;
	};
	curl_easy_cleanup(curlp);
	fclose(outfile);

}

void process_query( int id, shared_ptr<rxcpp::Observer<OBS>> obs, string vo_service, string url, rxcpp::Scheduler::shared scheduler ) {
        xml2::sax sax;
        string type;
        deque<string> keys_pattern;
        deque<string> keys;           // >>> used for setting up data maps
        map<string,string> values;
        bool collect_characters = false;
        bool inside_table = false;
        string characters;
		string within_field = "";

		// xmlStopParser(ctxt)
        sax.table.startElementNs =
            [&]( string name, string prefix, string uri, vector<tuple<string,string>> namespaces,
                 vector<tuple<string,string,string,string>> attributes ) {
                    if ( name == u8"TR" ) { if ( inside_table ) { type = "data"; keys = keys_pattern; values.clear(); } }
                    else if ( name == u8"TD" ) {  if ( inside_table ) { characters = ""; collect_characters = true; } }
                    else if ( name == u8"RESOURCE" ) { type = "begin"; values.clear(); }
                    else if ( name == u8"INFO" && attributes.size( ) > 1 ) {
                        if ( get<0>(attributes[0]) == u8"name" && get<0>(attributes[1]) == u8"value" )
                            values[get<3>(attributes[0])] = get<3>(attributes[1]);
                        else if ( get<0>(attributes[0]) == u8"value" && get<0>(attributes[1]) == u8"name" )
                            values[get<3>(attributes[1])] = get<3>(attributes[0]);
                    }
                    else if ( name == u8"TABLE" ) {
                        // >>>>===>> push out "begin" observable
                        values["URL base"] = vo_service;
                        values["URL full"] = url;
                        obs->OnNext(OBS( type, id, vo_service, values ));
                        type = "description";
                        values.clear( );
                        keys_pattern.clear( );
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
                        values[key] = typ + ":@:" + units + ":@:" + size + ":@:" + desc;
						within_field = key;
                    }
                    else if ( name == u8"DATA" ) {
                        // >>>>===>> push out "description" observable
                        obs->OnNext(OBS( type, id, vo_service, values ));
                        // "data" state is setup in <TR> and pushed out in </TR>...
                    }
                    else if ( name == u8"TABLEDATA" ) { inside_table = true; }
					else if ( within_field.size( ) > 0 && name == u8"DESCRIPTION" ) { characters = ""; collect_characters = true;; }
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
                    } else if ( name == u8"TR" ) {
                        if ( inside_table ) {
                             // >>>>===>> push out "description" observable
                             obs->OnNext(OBS( type, id, vo_service, values ));
                        }
                    } else if ( name == u8"DATA" ) {
                        // >>>>===>> signal completion
                        obs->OnNext(OBS( "end", id, vo_service, map<string,string>( ) ));
                        obs->OnCompleted( );
                    } else if ( name == u8"DESCRIPTION" && collect_characters ) {
						if ( within_field.size( ) > 0 ) {
							values[within_field] += ":@:" + characters;
						}
						collect_characters = false;
					}
            };

        sax.table.characters = [&]( string chars ) { if ( collect_characters ) characters += chars; };

        sax.table.warning = [&](string msg) {
			map<string,string> err;
			err["msg"] = msg;
			obs->OnNext(OBS( "warning", id, vo_service, err ));
		};
        sax.table.error = [&](string msg) {
			map<string,string> err;
			err["msg"] = msg;
			/**************  These could be handled by observing functions...  **************/
			obs->OnNext(OBS( "error", id, vo_service, err ));
			obs->OnNext(OBS( "end", id, vo_service, map<string,string>( ) ));

			/**************  OnError(...) terminates the Observable...         **************/
			auto excp = std::make_exception_ptr(std::runtime_error(msg));
			obs->OnError(excp);
		};

        auto ctxt = xmlCreatePushParserCtxt( sax.handler( ), &sax, nullptr, 0, nullptr);

		if ( util::exists( url ) ) {
			cout << "using local file " << url << endl;
			ifstream input( url );
			char buffer[1024];
			while ( input.read(buffer,sizeof(buffer)) || input.gcount( ) != 0 ) {
				if ( xmlParseChunk( ctxt, buffer, input.gcount( ), 0 ) ) {
					xmlParserError( ctxt, "xmlParseChunk" );
					break;
				}
			}
		} else {
			auto cb = function<curl::write_t>(
				   [&](void *ptr, size_t size, size_t nmemb) -> size_t {
	                    xmlParseChunk(ctxt, (const char*) ptr, size*nmemb, 0);
						return size*nmemb;
				   });
			CURL *curl = curl_easy_init( );
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str( ));
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, pvt::curlwrite);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &cb);
			curl_easy_perform(curl);
			curl_easy_cleanup(curl);
		}
		xmlFreeParserCtxt(ctxt);

    }

    adaptor::adaptor( string bus_name, string object_path ) : casa::dbus::address(bus_name,false),
                                                                        DBus::ObjectAdaptor( casa::DBusSession::instance( ).connection( ), object_path ),
																		standard_vos{ "http://vaosa-vm1.aoc.nrao.edu/ivoa-dal/siapv2-vao"
																		// standard_vos{ "http://vaosa-vm1.aoc.nrao.edu/ivoa-dal/siapv2"
		/*****************************/
		/* (1) "http://hla.stsci.edu/cgi-bin/hlaSIAP.cgi?imagetype=best&amp;inst=ACS,ACSGrism,WFC3,WFPC2,NICMOS,NICGRISM,COS,STIS,FOS,GHRS&amp;proprietary=false&amp;"  */
		/*     Overflow: Format must be image/fits,application/tar,text/html for all-sky search. Large area searches (r > 45 deg) are limited to a single instrument. Use advanced search options to change selected instruments. */
        /* (2) "http://hla.stsci.edu/cgi-bin/hlaSIAP.cgi?imagetype=best&amp;inst=NICMOS&amp;proprietary=false&amp;" */
		/*     Overflow: Format must be image/fits,application/tar,text/html for all-sky search. */
        /* (3) "http://hla.stsci.edu/cgi-bin/hlaSIAP.cgi?imagetype=image/fits&amp;inst=NICMOS&amp;proprietary=false&amp;" */
		/*     Overflow: Format must be image/fits,application/tar,text/html for all-sky search. */
        /* (4) "http://hla.stsci.edu/cgi-bin/hlaSIAP.cgi?imagetype=image/fits,application/tar,text/html&amp;inst=NICMOS&amp;proprietary=false&amp;" */
        /*     Overflow: Format must be image/fits,application/tar,text/html for all-sky search. */
		/* (1) http://irsa.ipac.caltech.edu/cgi-bin/Atlas/nph-atlas?mission=SWIRE&amp;hdr_location=%5CSWIREDataPath%5C&amp;collection_desc=The+Spitzer+Wide-area+InfraRed+Extragalactic+Survey+%28SWIRE%29&amp;SIAP_ACTIVE=1&amp;/sync?REQUEST=queryData&POS=180.000,0.000&SIZE=360.000,360.000" */
		/*     No images or sources were found for location: 00h 42m 44.33s +41d 16m 08.5s Eq J2000 */

		/* -------------------------------------------------------------------------------- */
		/* "http://cda.harvard.edu/cxcsiap/queryImages?" */
		/******************************/ }, last_id(0) { }

    adaptor::~adaptor( ) {
         /* signal: disconnect( ); */
    }

   int32_t adaptor::fetch(const string& url, const string &output, const bool& progress) {
		int32_t result = ++last_id;
		string source;
		if ( mode( ) == Mode::testing && util::exists( fetch_result_file ) ) {
			// likely will need to improve this to convert fetch_result_file to a fully qualified path
			source = "file://" + fetch_result_file;
		} else {
			source = url;
		}

		cout << "fetch url:\t" << source << endl;
		cout << "fetch out:\t" << output << endl;
		auto obs = create_subject( result, process_fetch, source, output, progress );
        map<string,function<void(int,string,string,double,double,double,double)>> signals {
            { "progress", [=]( int id, string path, string, double total, double done, double ultotal, double uldone) { this->fetch_progress(id,path,total,done,ultotal,uldone);} },
			{ "complete", [=]( int id, string path, string, double, double, double, double ) {this->fetch_complete(id,path);} },
			{ "error",    [=]( int id, string path, string err, double, double, double, double ) {this->fetch_error(id,path,err);} } };

		try {
// parallel queries...
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// waits for the new thread to exit.
			// rxcpp::from(obs).for_each( [&]( PROGRESS val ) {
			// 		signals[get<0>(val)](get<1>(val),get<2>(val),get<3>(val),get<4>(val),get<5>(val),get<6>(val),get<7>(val));
			// 	} );
			std::unique_lock<std::mutex> lock(pending_);
			pending.emplace( std::make_pair( result,
											 rxcpp::from(obs).subscribe(
												// on next
												[=]( PROGRESS val ) {
													signals.at(get<0>(val))(get<1>(val),get<2>(val),get<3>(val),get<4>(val),get<5>(val),get<6>(val),get<7>(val));
												},
												// on complete
												[&] {
													std::unique_lock<std::mutex> lock(pending_);
													auto it = pending.find(result);
													if ( it != pending.end( ) ) pending.erase(it);
												},
												// on error
												[&]( const std::exception_ptr &e ) {
													std::unique_lock<std::mutex> lock(pending_);
													auto it = pending.find(result);
													if ( it != pending.end( ) ) pending.erase(it);
												}
											 ) )
						   );
		} catch(...) {
			cout << "URL#2: " << url << endl;
			fprintf( stderr, "-----  -----  -----  -----  -----  -----  caught exception  -----  -----  -----  -----  -----  -----\n" );
		}

        return result;
    }

    int32_t adaptor::query( const double& ra, const double& dec,
                            const double& ra_size, const double& dec_size,
                            const string& format,
                            const map< string, ::DBus::Variant >& params,
                            const vector< string >& vos) {

        // set up the initial query parameters...
		auto qry = dal::Query( vos.size( ) > 0 ? vos[0] : standard_vos[0], ra, dec, ra_size, dec_size, format );

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

        map<string,function<void(int,string,const map<string,string>&)>> signals {
            { "begin", [=]( int id, string service, const map<string,string> &values ) {this->query_begin(id,service,values);} },
			{ "description", [=]( int id, string service, const map<string,string> &values ) {this->query_description(id,service,values);} },
			{ "data", [=]( int id, string service, const map<string,string> &values ) {this->query_data(id,service,values);} },
				{ "end", [=]( int id, string service, const map<string,string> &values ) { this->query_end(id,service,values);} },
			{ "warning", [=]( int id, string service, const map<string,string> &values ) {this->query_warning(id,service,values.at("msg"));} },
			{ "error", [=]( int id, string service, const map<string,string> &values ) {this->query_error(id,service,values.at("msg"));} } };

		cout << "query url:\t" << qry.url() << endl;
		// could run two queries each on a separate thread... (last_id should *ONLY* be incremented once...)
		int32_t result = ++last_id;

		auto obs1 = create_subject( result, process_query, standard_vos[0],
									mode( ) == Mode::testing && util::exists( query_result_file ) ? query_result_file : qry.url( ) );

// parallel queries...
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
//      auto obs2 = create_subject( ++last_id, process_query, standard_vos[0], qry.url( ) );
        // rxcpp::from(obs1).merge(obs2).for_each( [&]( OBS val ) {
        //           cout << "\t" << get<0>(val) << endl;
        //           signals[get<0>(val)](get<1>(val),get<2>(val),get<3>(val));
        // } );

// parallel queries...
// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
// waits for the new thread to exit.
//      rxcpp::from(obs1).for_each( [&]( OBS val ) {
//                signals[get<0>(val)](get<1>(val),get<2>(val),get<3>(val));
//      } );
        // process_query( ++last_id, qry.url( ) );
		try {
			std::unique_lock<std::mutex> lock(pending_);
		// schedules the subscription on the thread that is consumed by curl/libxml2 (thus it hangs)
		// ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- ----- -----
		//		rxcpp::from(obs1).subscribe_on(scheduler).
			pending.emplace( std::make_pair( result,
											 rxcpp::from(obs1).subscribe(
												// on next
												[=]( OBS val ) {
													signals.at(get<0>(val))(get<1>(val),get<2>(val),get<3>(val));
												},
												// on complete
												[&] {
													std::unique_lock<std::mutex> lock(pending_);
													auto it = pending.find(result);
													if ( it != pending.end( ) ) pending.erase(it);
												},
												// on error
												[&]( const std::exception_ptr &e ) {
													std::unique_lock<std::mutex> lock(pending_);
													auto it = pending.find(result);
													if ( it != pending.end( ) ) pending.erase(it);
												}
											 ) )
						   );
		} catch(...) {
			cout << "QUERY: " << endl;
			fprintf( stderr, "-----  -----  -----  -----  -----  -----  caught exception  -----  -----  -----  -----  -----  -----\n" );
		}

        return result;
    }

	void adaptor::cancel( const int32_t &id ) {
		std::unique_lock<std::mutex> lock(pending_);
		auto it = pending.find(id);
		if ( it != pending.end( ) ) {
			it->second.Dispose( );
			pending.erase(it);
		}
	}
}
