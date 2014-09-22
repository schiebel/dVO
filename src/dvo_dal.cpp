//# dvo_dal.cpp:  C++ interface to the URL creation routines of the VO Client DAL
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
#include <sstream>
#include <dvo/dal.hpp>

namespace dvo {
    namespace dal {
         struct query_kernel {
            query_kernel( const char *s, const char *v,
                          double ra, double dec, double ra_size,
                          double dec_size, const char *format );
			~query_kernel( );
            std::string url( ) const;
            void add( const char *key, long value );
            void add( const char *key, double value );
            void add( const char *key, const char *value );
            int dal;
            int query;              
        };

        Query::Query( std::string service, double ra, double dec, double ra_size, double dec_size, std::string format ) :
              dal{std::make_shared<query_kernel>( service.c_str( ),saip_version.c_str( ),
                                                  ra,dec,ra_size,dec_size,format.c_str( ) ) } { }

        std::string Query::url( ) const { return dal->url( ); }

        Query Query::add( std::string key, long value ) {
            dal->add( key.c_str( ), value );
            return *this;
        }
        Query Query::add( std::string key, double value ) {
            dal->add( key.c_str( ), value );
            return *this;
        }
        Query Query::add( std::string key, std::string value) {
            dal->add( key.c_str( ), value.c_str( ) );
            return *this;
        }


    }
}

#define VOC_DIRECT
#include <voc/VOClient.h>

// VOClient seems to not modify strings, but also does not declare them const...
// should it be discovered that they are modified, we would need to create a copy...
dvo::dal::query_kernel::query_kernel( const char *s, const char *v, double ra, double dec,
                                      double ra_size, double dec_size, const char *format ) {
    dal = voc_openSiapConnection((char*)s,(char*)v);
    if ( dal == DAL_ERROR ) {
        std::ostringstream o;
        o << "service connection failed: " << voc_getError(dal);
        throw std::runtime_error(o.str( ));
    }
    query = voc_getSiapQuery( dal, ra, dec, ra_size, dec_size, strlen(format) == 0 ? 0 : (char*) format );
    if ( query == DAL_ERROR ) {
        std::ostringstream o;
        o << "query construction failed: " << voc_getError(dal);
        throw std::runtime_error(o.str( ));
    }
}

dvo::dal::query_kernel::~query_kernel( ) {
	fflush(stderr);
	voc_closeConnection(dal);
}

std::string dvo::dal::query_kernel::url( ) const { return std::string(voc_getQueryURL(query)); }

void dvo::dal::query_kernel::add( const char *key, long value ) {
    voc_addIntParam( query, (char*) key, value );
}
void dvo::dal::query_kernel::add( const char *key, double value ) {
    voc_addFloatParam( query, (char*) key, value );
}
void dvo::dal::query_kernel::add( const char *key, const char *value ) {
    voc_addStringParam( query, (char*) key, (char*) value );
}
