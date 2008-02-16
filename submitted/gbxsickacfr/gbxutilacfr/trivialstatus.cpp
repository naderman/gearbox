/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <sstream>
#include "trivialstatus.h"

using namespace std;

namespace gbxsickacfr {
namespace gbxutilacfr {

TrivialStatus::TrivialStatus( Tracer& tracer,
        bool heartbeat, bool ok, bool init, bool warn, bool fault ) : 
    tracer_(tracer),
    heartbeat_(heartbeat),
    ok_(ok),
    init_(init),
    warn_(warn),
    fault_(fault)
{
}

void 
TrivialStatus::addSubsystem( const std::string& subsystem, double maxHeartbeatIntervalSec )
{
    stringstream ss;
    ss << "TrivialStatus::setMaxHeartbeatInterval(): Adding new subsystem: '"<<subsystem<<"'";
    tracer_.debug( ss.str() );
}

void 
TrivialStatus::removeSubsystem( const std::string& subsystem )
{
    stringstream ss;
    ss << "TrivialStatus::removeSubsystem(): Removing existing subsystem: '"<<subsystem<<"'";
    tracer_.debug( ss.str() );
}

void 
TrivialStatus::setMaxHeartbeatInterval( const std::string& subsystem, double maxHeartbeatIntervalSec )
{
}

void 
TrivialStatus::initialising( const std::string& subsystem, const std::string& message )
{
    if ( init_ && !message.empty() )
        tracer_.info( "TrivialStatus: initialising subsystem "+subsystem+": '"+message+"'" );
}

void 
TrivialStatus::ok( const std::string& subsystem, const std::string& message )
{
    if ( ok_ && !message.empty() )
        tracer_.info( "TrivialStatus: "+subsystem+" is ok : '"+message+"'" );
}

void 
TrivialStatus::warning( const std::string& subsystem, const std::string& message )
{
    if ( warn_ )
        tracer_.warning( "TrivialStatus: "+subsystem+" issued warning : '"+message+"'" );
}

void 
TrivialStatus::fault( const std::string& subsystem, const std::string& message )
{
    if ( fault_ )
        tracer_.error( "TrivialStatus: "+subsystem+" issued fault : '"+message+"'" );
}
    
void 
TrivialStatus::heartbeat( const std::string& subsystem )
{
    if ( heartbeat_ )
        tracer_.info( "TrivialStatus: heartbeat for subsystem "+subsystem );
}

void 
TrivialStatus::process()
{
}

}
}
