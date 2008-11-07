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
    
    virtual void addSubsystem( const std::string& subsystem, 
            double maxHeartbeatIntervalSec=-1.0, SubsystemType type=SubsystemStandard );
    virtual void removeSubsystem( const std::string& subsystem );
    //! Does not keep track of subsystems, returns empty vector.
    virtual std::vector<std::string> subsystems();
    //! Does not keep track of status, throws Exception on any query
    virtual SubsystemStatus subsystemStatus( const std::string& subsystem );
    //! Does not keep track of infrastructure state, throws Exception on any query
    virtual SubsystemState infrastructureState();
    virtual void setMaxHeartbeatInterval( const std::string& subsystem, double interval );
    virtual void setSubsystemType( const std::string& subsystem, SubsystemType type );

    virtual void setSubsystemStatus( const std::string& subsystem, SubsystemState state, SubsystemHealth health, const std::string& msg="" );

    virtual void initialising( const std::string& subsystem, const std::string& msg="" );
    virtual void working( const std::string& subsystem, const std::string& msg="" );
    virtual void finalising( const std::string& subsystem, const std::string& msg="" );

    virtual void ok( const std::string& subsystem, const std::string& msg="" );
    virtual void warning( const std::string& subsystem, const std::string& msg );
    virtual void fault( const std::string& subsystem, const std::string& msg );
    virtual void heartbeat( const std::string& subsystem );
    virtual void message( const std::string& subsystem, const std::string& msg );

    virtual void infrastructureInitialising();
    virtual void infrastructureWorking();
    virtual void infrastructureFinalising();
    virtual void process();

private:

    void internalSetStatus( const std::string& subsystem, gbxutilacfr::SubsystemState state, 
                gbxutilacfr::SubsystemHealth health, const std::string& msg="" );

    Tracer& tracer_;

    bool stateChange_;
    bool ok_;
    bool warn_;
    bool fault_;
    bool heartbeat_;
};

} // namespace

#endif
