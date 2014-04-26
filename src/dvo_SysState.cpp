//# dvo_SysState.cpp: create observable of system states (sleep/wake)
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

//    For reference see: TECHNICAL Q&A  QA1340
//    http://developer.apple.com/qa/qa2004/qa1340.html

//       compile with e.g. clang++ -std=gnu++11 -stdlib=libc++ -I.... -o EXE dvo_SysState.cpp -framework CoreFoundation -lIOKit

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#include <mach/mach_port.h>
#include <mach/mach_interface.h>
#include <mach/mach_init.h>

#include <IOKit/pwr_mgt/IOPMLib.h>
#include <IOKit/IOMessage.h>

#include <dvo/SysState.hpp>

using std::shared_ptr;
using std::function;

namespace dvo {

	static io_connect_t  root_port = 0;

	shared_ptr<rxcpp::Observable<SysState>> SysStateObservable( rxcpp::Scheduler::shared scheduler ) {
		static auto subject = rxcpp::CreateSubject<SysState>( );
		// multiple requests can share the same Observable...
		if( root_port != 0 ) return subject;

		if ( ! scheduler ) {
			scheduler = std::make_shared<rxcpp::EventLoopScheduler>( );
		}

		struct {
			static void cfunc( void *data, io_service_t y, natural_t messageType, void * messageArgument ) {
				rxcpp::Subject<SysState> *obs = static_cast<rxcpp::Subject<SysState>*>(data);

				switch( messageType ) {
				case kIOMessageSystemWillSleep:
					obs->OnNext(SysState::WillSleep);
					IOAllowPowerChange( root_port, (long) messageArgument );
					break;
				case kIOMessageCanSystemSleep:
					//       System would like to start Energy Saver idle sleep...
					//       an application can cancel sleep with:
					//IOCancelPowerChange(root_port, (long)messageArgument);
					//       but most application should avoid this...
					//       The Power Manager waits up to 30 seconds for a reply..
					obs->OnNext(SysState::MaySleep);
					IOAllowPowerChange( root_port, (long) messageArgument );
					break;

				case kIOMessageSystemWillPowerOn:
					obs->OnNext(SysState::WillPowerOn);
					break;
				case kIOMessageSystemWillRestart:
					break;
				case kIOMessageSystemHasPoweredOn:
					obs->OnNext(SysState::HasPoweredOn);
					break;
				}
			}
		} callback;

	    scheduler->Schedule( rxcpp::fix0(
			[=]( rxcpp::Scheduler::shared s, function<rxcpp::Disposable(rxcpp::Scheduler::shared)> self) -> rxcpp::Disposable {
				IONotificationPortRef	notify;
				io_object_t				iter;
				// subject is static...
				root_port = IORegisterForSystemPower( subject.get( ), &notify, callback.cfunc, &iter );
				if( root_port != 0 ) {
					CFRunLoopAddSource( CFRunLoopGetCurrent(),
										IONotificationPortGetRunLoopSource(notify),
										kCFRunLoopCommonModes );
					// if a Carbon or Cocoa event loop is already running another one does not need to be started...
					CFRunLoopRun();
				}
				return rxcpp::Disposable::Empty( );
			} ) );
		return subject;
	}

}
