/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXUTILACFR_STATUS_H
#define GBXUTILACFR_STATUS_H

#include <string>
#include <vector>

namespace gbxutilacfr {

//! Possible subsystem status values
enum SubsystemState
{
    //! Subsystem has been created but has not started initialisation process.
    SubsystemIdle,
    //! Subsystem is preparing to work, e.g. initialising its resources, etc.
    SubsystemInitialising,
    //! Subsystem is fully initialised and is performing its function.
    SubsystemWorking,
    //! Subsystem is preparing to shutdown, e.g. releasing its resources, etc.
    SubsystemFinalising,
    //! Subsystem is not longer functioning.
    SubsystemShutdown
};

//! Returns string equivalent of state enumerator.
std::string toString( SubsystemState state );

//! Possible subsystem status values
enum SubsystemHealth
{
    //! Subsystem is OK
    SubsystemOk,
    //! Subsystem has encountered an abnormal but non-fault condition
    SubsystemWarning,
    //! Subsystem has declared a fault
    SubsystemFault,
    //! Subsystem has not been heard from for an abnormally long time
    SubsystemStalled
};

//! Returns string equivalent of health enumerator.
std::string toString( SubsystemHealth health );

//! Status for a single subsystem
struct SubsystemStatus
{
    //! Constructor.
    SubsystemStatus( SubsystemState s=SubsystemIdle, SubsystemHealth h=SubsystemOk, const std::string& msg="", double beat=0.0 ) :
        state(s),
        health(h),
        message(msg),
        sinceHeartbeat(beat) {};

    //! Current state in the subsystem's state machine. I.e. what is the subsystem doing?
    SubsystemState state;

    //! Subsystem's health. I.e. how is the subsystem doing?
    SubsystemHealth health;

    //! Human-readable status description
    std::string message;

    //! Ratio of time since last heartbeat to maximum expected time between heartbeats.
    //! For example, sinceHeartbeat=0.5 means that half of normally expected interval between heartbeats
    //! has elapsed.
    double sinceHeartbeat;
};

//! Returns human-readable string with subsystem status information.
std::string toString( const SubsystemStatus& status );

/*!
@brief Local interface to component status.

@par Overview

Status provides a machine-readable interface such that tools external
to the component can monitor its status. A single Status object is meant
to be shared by all threads in the library, so the implementation must
be thread-safe. The idea is that Status tracks the state of a number
of subsystems (most often one per thread).

Each subsystem should first call addSubsystem(), to make the
Status engine aware that it exists. If any other function is called before 
the subsystem is added, a gbxutilacfr::Exception is thrown.

The default initial status of a subsystem is @c Idle with health @c OK.

After registering a subsystem, a subsystem can report its state and health.
Each of the calls is sufficient to let
the Status engine know that the subsystem is alive.  The special call
'heartbeat' lets Status know that the subsystem is alive without
modifying its status.

The 'maxHeartbeatIntervalSec' parameter tells the Status engine how often it 
should expect to hear from the subsystem.  If no message is received from a subsystem for
longer than @c maxHeartbeatIntervalSec, it is assumed that the subsystem has stalled (hung).

@par State Machine

The state machine of a subsystem is a chain of state transitions with one extra link:
@verbatim
Idle --> Initialising --> Working --> Finalising --> Shutdown
              |___________________________^
@endverbatim
The following represents the Subsystem state machine in the format of
State Machine Compiler (see smc.sf.net) :
@verbatim
Idle
Entry { init(); }
{
    init
    Initialising
    {}
}

Initialising
Entry { initialise(); }
{
    [ !isStopping ] finished
    Working
    {}

    [ isStopping ] finished
    Finalising
    {}
}

Working
Entry { work(); }
{
    finished
    Finalising
    {}
}

Finalising
Entry { finalise(); }
{
    finished
    Shutdown
    {}
}

Shutdown
{
}   
@endverbatim

@sa Tracer
@sa SubStatus
*/
class Status
{

public:

    virtual ~Status() {};

    /*!
    Adds subsystem to the system status monitor. This command must be called before any
    other. I.e. all other status commands are ignored unless a subsystem with that name
    already exists. When trying to add a subsystem with an existing name, the existing
    subsystem is left unchanged and warning trace is produced.
    
    May also specify the maximum expected interval between heartbeats. 
    When time since last heartbeat exceeds this, an alarm is raised. Heartbeat interval is normally
    positive, measured in seconds. Negative interval means infinite interval, this is the default behavior.
    
    The initial state of the subsystem is Initialising.
    */
    virtual void addSubsystem( const std::string& subsystem, double maxHeartbeatIntervalSec=-1.0 )=0;

    //! Removes subsystem from the status monitor. This should be done for example, if one of
    //! the thread is shutting down or restarting. 
    //! Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void removeSubsystem( const std::string& subsystem )=0;

    //! Returns a list of subsystem names.
    virtual std::vector<std::string> subsystems()=0;

    //! Returns status of subsystem with the given name.
    //! Throws gbxutilacfr::Exception when the specified subsystem does not exist.
    virtual SubsystemStatus subsystemStatus( const std::string& subsystem )=0;

    //! Modifies maximum expected interval between heartbeats (in seconds).
    //! When time since last heartbeat exceeds this, alarm is raised. 
    //! Negative interval means infinite interval.
    //! Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void setMaxHeartbeatInterval( const std::string& subsystem, double intervalSec )=0;

    //
    // BOTH STATE AND HEALTH CHANGES
    //

    //! Sets the status of a subsystem (both state and health) in an atomic operation. Use this method
    //! when both state and health have changed.
    virtual void setSubsystemStatus( const std::string& subsystem, SubsystemState state, SubsystemHealth health, const std::string& message="" )=0;

    //
    // STATE CHANGES
    //

    //! Sets state of the subsystem to Initialising. Note that empty message is assumed if none is supplied.
    //! Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void initialising( const std::string& subsystem, const std::string& message="" )=0;
    //! Sets state of the subsystem to Working. Note that empty message is assumed if none is supplied.
    //! Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void working( const std::string& subsystem, const std::string& message="" )=0;
    //! Sets state of the subsystem to Finalising. Note that empty message is assumed if none is supplied.
    //! Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void finalising( const std::string& subsystem, const std::string& message="" )=0;

    //
    // HEALTH CHANGES
    //
    
    //! Sets subsystem health to Ok. Note that empty message is assumed if none is supplied.
    //! Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void ok( const std::string& subsystem, const std::string& message="" )=0;

    //! Sets subsystem health to Warning.
    //! Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void warning( const std::string& subsystem, const std::string& message )=0;

    //! Sets subsystem health to Fault.
    //! Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void fault( const std::string& subsystem, const std::string& message )=0;

    //
    // NO CHANGE
    //

    //! Record heartbeat from a subsystem: let Status know the subsystem is alive without
    //! modifying its status.
    //! Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void heartbeat( const std::string& subsystem )=0;

    //
    // Utility
    //

    //! Some thread must call this function periodically in order for
    //! status publishing to happen and stalls identified.
    virtual void process()=0;
};

} // namespace

#endif
