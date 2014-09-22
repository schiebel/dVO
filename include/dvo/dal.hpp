//# dal.hpp: C++ interface to C VOClient DAL URL functions
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
#include <string>

namespace dvo {
    namespace dal {
        struct query_kernel;
        class Query {
        public:
             Query( std::string service, double ra, double dec, double ra_size, double dec_size, std::string format="" );
             Query( const Query &that ) : dal{that.dal} { }
             Query &operator=( const Query that ) { dal = that.dal; return *this; }
             Query( Query &&that ) : dal{std::move(that.dal)} { }
             Query add( std::string key, long value );
             Query add( std::string key, double value );
             Query add( std::string key, std::string value);
             std::string url( ) const;
        private:
            const std::string saip_version{"2.0p"};
            std::shared_ptr<query_kernel> dal;
        };
    }
}
