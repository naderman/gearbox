/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXUTILACFR_SUBSYSTEM_STATUS_H
#define GBXUTILACFR_SUBSYSTEM_STATUS_H

#include <gbxutilacfr/status.h>

namespace gbxutilacfr {

//!
//! @brief Convenience class which represents the status of a subsystem.
//!
//! @par Overview
//!
//! SubStatus provides a machine-readable interface such that other components can 
//! monitor this component's status.
//!
//! A single SubStatus object is meant to be shared by all threads in the component so the
//! implementation must be thread-safe.
//!
//!
//! @sa SubStatus
//!
class SubStatus
{

public:
    //! Sets a reference to the system Status and this subsystem's name.
    //! Adds this subsystem to the system with default (infinite) maximum heartbeat interval.
    SubStatus( Status& status, const std::string& subsystem ) :
        status_(status),
        subsysName_(subsystem) 
    {
        status_.addSubsystem( subsysName_ );
    };

    //! Removes this subsystem from the system.
    ~SubStatus()
    {
        status_.removeSubsystem( subsysName_ );
    }

    //! Passes this information to the system Status.
    void setMaxHeartbeatInterval( double interval ) { status_.setMaxHeartbeatInterval( subsysName_, interval ); };

    //! Passes this information to the system Status.
    void heartbeat() { status_.heartbeat( subsysName_ ); };

    //! Passes this information to the system Status.
    void initialising( const std::string& message="" ) { status_.initialising( subsysName_, message ); };

    //! Passes this information to the system Status.
    void ok( const std::string& message="" ) { status_.ok( subsysName_, message ); };

    //! Passes this information to the system Status.
    void warning( const std::string& message ) { status_.warning( subsysName_, message ); };

    //! Passes this information to the system Status.
    void fault( const std::string& message ) { status_.fault( subsysName_, message ); };

    //! Returns subsystem's name
    std::string name() const { return subsysName_; };
private:

    Status& status_;
    std::string subsysName_;
};

} // namespace

#endif
