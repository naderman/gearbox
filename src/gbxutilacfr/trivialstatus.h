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
//! @brief A trivial implementation of the status API which does not assemble information.
//!
//! System status information is not assembled but all changes are traced to Tracer.
//!
//! @see Status
//!
class TrivialStatus : public Status
{
public:

    TrivialStatus( Tracer& tracer, 
        bool stateChange=true, bool ok=false, bool warn=true, bool fault=true, bool heartbeat=false );
    
    virtual void addSubsystem( const std::string& subsystem, double maxHeartbeatIntervalSec=-1.0 );
    virtual void removeSubsystem( const std::string& subsystem );
    //! does not keep track of subsystems, returns empty vector.
    virtual std::vector<std::string> subsystems();
    //! does not keep track of status, throws Exception on any query
    virtual SubsystemStatus subsystemStatus( const std::string& subsystem );
    virtual void setMaxHeartbeatInterval( const std::string& subsystem, double interval );

    virtual void setSubsystemStatus( const std::string& subsystem, SubsystemState state, SubsystemHealth health, const std::string& message="" );

    virtual void initialising( const std::string& subsystem, const std::string& message="" );
    virtual void working( const std::string& subsystem, const std::string& message="" );
    virtual void finalising( const std::string& subsystem, const std::string& message="" );

    virtual void ok( const std::string& subsystem, const std::string& message="" );
    virtual void warning( const std::string& subsystem, const std::string& message );
    virtual void fault( const std::string& subsystem, const std::string& message );
    virtual void heartbeat( const std::string& subsystem );

    virtual void process();

private:

    Tracer& tracer_;

    bool stateChange_;
    bool ok_;
    bool warn_;
    bool fault_;
    bool heartbeat_;
};

} // namespace

#endif
