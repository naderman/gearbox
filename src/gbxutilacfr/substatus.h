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

#if defined (WIN32)
    #if defined (GBXUTILACFR_STATIC)
        #define GBXUTILACFR_EXPORT
    #elif defined (GBXUTILACFR_EXPORTS)
        #define GBXUTILACFR_EXPORT       __declspec (dllexport)
    #else
        #define GBXUTILACFR_EXPORT       __declspec (dllimport)
    #endif
#else
    #define GBXUTILACFR_EXPORT
#endif

#include <gbxutilacfr/status.h>

namespace gbxutilacfr {

//!
//! @brief Convenience class which maniupulates the status of a subsystem.
//!
//! @par Overview
//!
//! Provides a convenient interface for setting status information for one subsystem.
//!
//! @sa Status, @sa SubHealth
//!
class GBXUTILACFR_EXPORT SubStatus
{

public:
    //! Sets a reference to the system Status and this subsystem's name.
    //! Adds this subsystem to the system.
    SubStatus( Status& status, const std::string& subsysName, double maxHeartbeatIntervalSec=-1.0 ) :
        status_(status),
        subsysName_(subsysName) 
    {
        status_.addSubsystem( subsysName_, maxHeartbeatIntervalSec );
    };

    //! Removes this subsystem from the system.
    ~SubStatus()
    {
        status_.removeSubsystem( subsysName_ );
    }

    //
    // set expectations about ourselves
    //

    //! Passes this information to the system Status.
    void setMaxHeartbeatInterval( double interval ) { status_.setMaxHeartbeatInterval( subsysName_, interval ); };

    //! Passes this information to the system Status.
    void setSubsystemType( SubsystemType type ) { status_.setSubsystemType( subsysName_, type ); };

    //
    // set health
    //

    //! Passes this information to the system Status.
    void heartbeat() { status_.heartbeat( subsysName_ ); };

    //! Passes this information to the system Status.
    void message( const std::string& message ) { status_.message( subsysName_, message ); };

    //! Passes this information to the system Status.
    void ok( const std::string& message="" ) { status_.ok( subsysName_, message ); };

    //! Passes this information to the system Status.
    void warning( const std::string& message ) { status_.warning( subsysName_, message ); };

    //! Passes this information to the system Status.
    void fault( const std::string& message ) { status_.fault( subsysName_, message ); };

    //
    // Set state machine states
    //

    //! Passes this information to the system Status.
    void initialising( const std::string& message="" ) { status_.initialising( subsysName_, message ); };

    //! Passes this information to the system Status.
    void working( const std::string& message="" ) { status_.working( subsysName_, message ); };

    //! Passes this information to the system Status.
    void finalising( const std::string& message="" ) { status_.finalising( subsysName_, message ); };


    //! Returns system Status object
    Status& status() { return status_; };

    //! Returns subsystem's name
    std::string name() const { return subsysName_; };

private:

    Status& status_;
    std::string subsysName_;
};

} // namespace

#endif
