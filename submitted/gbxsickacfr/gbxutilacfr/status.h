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

namespace gbxsickacfr {
namespace gbxutilacfr {

/*
@brief Local interface to component status.

@par Overview

Status provides a machine-readable interface such that other components can 
monitor this component's status. A single Status object is meant to be shared by all 
threads in the component so the implementation must be thread-safe. The idea is that Status 
tracks the state of a number of subsystems (most often one per thread).

Each subsystem should first call addSubsystem(), to make the
Status engine aware that it exists. If any other function is called before 
the subsystem is added, a gbxutilacfr::Exception is thrown.

The 'maxHeartbeatIntervalSec' parameter tells the Status engine how often it expects to hear
from each subsystem.  If the subsystem has not been heard from for
longer than maxHeartbeatIntervalSec, it is assumed that the 
subsystem has stalled (hung).

The initial default state of Initialising. As soon as initialisation of the 
subsystem is finished, you should call ok(). This maybe used by external tools
as an indication that your subsystem is in "normal" working state.

Status will publish the entire status of every subsystem whenever
anything changes, or every @c Orca.Status.PublishPeriod, whichever
occurs first.

@par Local Calls

After registering with setMaxHeartbeatInterval, components set
their subsystems' status with the various calls.  Each of the calls
is sufficient to let the Status engine know that the subsystem is alive.
The special call 'heartbeat' lets Status know that the subsystem is
 alive without modifying its status.

@par Configuration parameters

- @c  Orca.Status.RequireIceStorm (bool)
    - gbxutilacfr::Component sets up a status and tries to connect to an
      IceStorm server on the same host in order to publish component's
      status messages. This parameter determines what happens if no server
      is found. If set to 0, the startup continues with status messages not
      published remotely. If set to 1, the application exits.
    - Default: 0

- @c  Orca.Status.PublishPeriod (double)
    - The minimum interval, in seconds, between remote publishing of status messages.
      The actual interval will be less if status changes.
    - Default: 30

@sa Tracer
*/
class Status
{

public:

    // this is for internal implementation only
    enum SubsystemStatusType
    {
        Initialising,
        Ok,
        Warning,
        Fault,
        Stalled
    };
    
    virtual ~Status() {};

    /*
    Adds subsystem to the system status monitor. This command must be called before any
    other. I.e. all other status commands are ignored unless a subsystem with that name
    already exists. When trying to add a subsystem with an existing name, the existing
    subsystem is left unchanged and warning trace is produced.
    
    May also specify the maximum expected interval between heartbeats. 
    When time since last heartbeat exceeds this, alarm is raised. Heartbeat interval is normally
    positive, measured in seconds. Negative interval means infinite interval, this is the default behavior.
    
    The initial state of the subsystem is Initialising.
    */
    virtual void addSubsystem( const std::string& subsystem, double maxHeartbeatIntervalSec=-1.0 )=0;

    // Removes subsystem from the status monitor. This should be done for example, if one of
    // the thread is shutting down or restarting. 
    // Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void removeSubsystem( const std::string& subsystem )=0;

    // Modifies maximum expected interval between heartbeats.
    // When time since last heartbeat exceeds this, alarm is raised. 
    // Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void setMaxHeartbeatInterval( const std::string& subsystem, double interval )=0;

    // Sets subsystem status to Initialising. Note that empty message is assumed if none is supplied.
    // Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void initialising( const std::string& subsystem, const std::string& message="" )=0;

    // Sets subsystem status to Ok. Note that empty message is assumed if none is supplied.
    // Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void ok( const std::string& subsystem, const std::string& message="" )=0;

    // Sets subsystem status to Warning.
    // Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void warning( const std::string& subsystem, const std::string& message )=0;

    // Sets subsystem status to Fault.
    // Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void fault( const std::string& subsystem, const std::string& message )=0;

    // Record heartbeat from a subsystem: let Status know the subsystem is alive without
    // modifying its status.
    // Throws gbxutilacfr::Exception if the subsystem does not exist.
    virtual void heartbeat( const std::string& subsystem )=0;

    // Some thread should call this function periodically in order for
    // status publishing to happen.
    virtual void process()=0;
};

}
} // namespace

#endif
