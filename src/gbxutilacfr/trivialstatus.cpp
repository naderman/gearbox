/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <sstream>
#include "trivialstatus.h"
#include "exceptions.h"
#include <assert.h>

using namespace std;

namespace gbxutilacfr {

TrivialStatus::TrivialStatus( Tracer& tracer,
        bool stateChange, bool ok, bool warn, bool fault, bool heartbeat ) :
    tracer_(tracer),
    stateChange_(stateChange),
    ok_(ok),
    warn_(warn),
    fault_(fault),
    heartbeat_(heartbeat)
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

std::vector<std::string> 
TrivialStatus::subsystems()
{
    return std::vector<std::string> ();
}

SubsystemStatus 
TrivialStatus::subsystemStatus( const std::string& subsystem )
{
    throw Exception( ERROR_INFO, "This implementation of Status does not store status of the subsystems" );
}

void 
TrivialStatus::setMaxHeartbeatInterval( const std::string& subsystem, double maxHeartbeatIntervalSec )
{
}

void 
TrivialStatus::setSubsystemStatus( const std::string& subsystem, SubsystemState state, SubsystemHealth health, const std::string& message )
{
    string trace = "TrivialStatus: subsystem "+subsystem+" changed state to "+gbxutilacfr::toString(state)+" with health "+gbxutilacfr::toString(health);
    if (!message.empty() )
        trace =+ ": '" + message + "'";
    tracer_.info( trace );
}

void 
TrivialStatus::initialising( const std::string& subsystem, const std::string& message )
{
    if ( stateChange_ ) {
        string trace = "TrivialStatus: subsystem "+subsystem+" changed state to Initialising";
        if (!message.empty() )
            trace =+ " and message: '" + message + "'";
        tracer_.info( trace );
    }
}

void 
TrivialStatus::working( const std::string& subsystem, const std::string& message )
{
    if ( stateChange_ ) {
        string trace = "TrivialStatus: subsystem "+subsystem+" changed state to Working";
        if (!message.empty() )
            trace =+ ": '" + message + "'";
        tracer_.info( trace );
    }
}

void 
TrivialStatus::finalising( const std::string& subsystem, const std::string& message )
{
    if ( stateChange_ ) {
        string trace = "TrivialStatus: subsystem "+subsystem+" changed state to Finalising";
        if (!message.empty() )
            trace =+ ": '" + message + "'";
        tracer_.info( trace );
    }
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
