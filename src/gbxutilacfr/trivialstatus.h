/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXUTILACFR_TRIVIAL_STATUS_H
#define GBXUTILACFR_TRIVIAL_STATUS_H

#include <gbxutilacfr/status.h>
#include <gbxutilacfr/tracer.h>

namespace gbxutilacfr {


//!
//! @brief A trivial implementation of the status API which prints to cout.
//!
//! @see Status
//!
class TrivialStatus : public Status
{
public:

    TrivialStatus( Tracer& tracer, 
        bool heartbeat=false, bool ok=false, bool init=false, bool warn=true, bool fault=true );
    
    virtual void addSubsystem( const std::string& subsystem, double maxHeartbeatIntervalSec=-1.0 );
    virtual void removeSubsystem( const std::string& subsystem );
    //! does not keep track of subsystems, returns empty vector.
    virtual std::vector<std::string> subsystems();
    //! does not keep track of status, throws Exception on any query
    virtual SubsystemStatus subsystemStatus( const std::string& subsystem );
    virtual void setMaxHeartbeatInterval( const std::string& subsystem, double interval );
    virtual void initialising( const std::string& subsystem, const std::string& message="" );
    virtual void ok( const std::string& subsystem, const std::string& message="" );
    virtual void warning( const std::string& subsystem, const std::string& message );
    virtual void fault( const std::string& subsystem, const std::string& message );
    virtual void heartbeat( const std::string& subsystem );
    virtual void process();

private:

    Tracer& tracer_;

    bool heartbeat_;
    bool ok_;
    bool init_;
    bool warn_;
    bool fault_;
};

} // namespace

#endif
