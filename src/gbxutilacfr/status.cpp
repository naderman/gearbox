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
        break;
    }
    return "Shutdown";
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
    case SubsystemStalled :
        break;
    }
    return "Stalled";
}

} // namespace
