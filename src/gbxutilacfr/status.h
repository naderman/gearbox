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
    SubsystemIdle,
    SubsystemInitialising,
    SubsystemWorking,
    SubsystemFinalising,
    SubsystemShutdown
};

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

std::string toString( SubsystemHealth health );

//! Status for a single subsystem
struct SubsystemStatus
{
    //! Machine-readable status description
    SubsystemState state;

    SubsystemHealth health;

    //! Human-readable status description
    std::string message;

    //! Ratio of time since last heartbeat to maximum expected time between heartbeats.
    //! For example, sinceHeartbeat=0.5 means that half of normally expected interval between heartbeats
    //! has elapsed.
    float sinceHeartbeat;
};

/*!
@brief Local interface to component status.

@par Overview

Status provides a machine-readable interface such that tools external
to the library can monitor its status. A single Status object is meant
to be shared by all threads in the library, so the implementation must
be thread-safe. The idea is that Status tracks the state of a number
of subsystems (most often one per thread).

Each subsystem should first call addSubsystem(), to make the
Status engine aware that it exists. If any other function is called before 
the subsystem is added, a gbxutilacfr::Exception is thrown.

The 'maxHeartbeatIntervalSec' parameter tells the Status engine how often it expects to hear
from each subsystem.  If the subsystem has not been heard from for
longer than maxHeartbeatIntervalSec, it is assumed that the 
subsystem has stalled (hung).

The initial default state is Initialising. As soon as initialisation
of the subsystem is finished, you should call ok(). This maybe used by
external tools as an indication that your subsystem is in "normal"
working state.

@par Local Calls

After registering with setMaxHeartbeatInterval, set the subsystems'
status with the various calls.  Each of the calls is sufficient to let
the Status engine know that the subsystem is alive.  The special call
'heartbeat' lets Status know that the subsystem is alive without
modifying its status.

@sa Tracer
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
    virtual void working( const std::string& subsystem, const std::string& message="" )=0;
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
