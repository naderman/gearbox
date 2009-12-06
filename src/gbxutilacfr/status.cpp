/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include "status.h"
#include <sstream>
#include <assert.h>

namespace gbxutilacfr {

std::string toString( SubsystemState state )
{
    switch ( state ) 
    {
    case SubsystemInitialising :
        return "Initialising";
    case SubsystemWorking :
        return "Working";
    case SubsystemFinalising :
        return "Finalising";
    case SubsystemIdle :
        return "Idle";
    case SubsystemShutdown :
        return "Shutdown";
    default:
        assert( !"gbxutilacfr::toString(SubsystemState) should never get to default" );
    }
	return ""; // Shuts up MSVC
}

std::string toString( SubsystemHealth health )
{
    switch ( health ) 
    {
    case SubsystemOk :
        return "Ok";
    case SubsystemWarning :
        return "Warning";
    case SubsystemFault :
        return "Fault";
    default:
        assert( !"gbxutilacfr::toString(SubsystemHealth) should never get to default" );
    }
	return ""; // Shuts up MSVC
}

std::string toString( const SubsystemStatus& status )
{
    std::stringstream ss;
    ss << "state="<<toString(status.state)
       << " health="<<toString(status.health)
       << " msg='"<<status.message<<"'"
       << " since hearbeat="<<status.sinceHeartbeat;
    return ss.str();
	return ""; // Shuts up MSVC
}

std::string toString( SubsystemType type )
{
    switch ( type ) 
    {
    case SubsystemStandard :
        return "Standard";
    case SubsystemEarlyExit :
        return "EarlyExit";
    default:
        assert( !"gbxutilacfr::toString(SubsystemType) should never get to default" );
    }
	return ""; // Shuts up MSVC
}

} // namespace
