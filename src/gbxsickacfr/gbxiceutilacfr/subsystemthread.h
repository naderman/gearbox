/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXICEUTILACFR_SUBSYSTEM_THREAD_H
#define GBXICEUTILACFR_SUBSYSTEM_THREAD_H

#include <gbxsickacfr/gbxiceutilacfr/thread.h>
#include <gbxsickacfr/gbxutilacfr/substatus.h>
#include <gbxsickacfr/gbxutilacfr/status.h>
#include <gbxsickacfr/gbxutilacfr/tracer.h>

namespace gbxsickacfr {
namespace gbxiceutilacfr {

/*!
@brief A version of the Thread class which catches all possible exceptions and integrates some Status operations.

If a stray exception is caught, an error message will be printed
(using cout), then we will wait for someone to call stop().

To use this class, simply implement the pure virtual walk() function.
@verbatim
void MyThread::walk()
{
    // initialize

    // main loop
    while ( !isStopping() )
    {
        // do something
    }

    // clean up
}
@endverbatim

@see Thread, SafeThread.
 */
class SubsystemThread : public Thread
{
public:
    //! Supply an optional Tracer and Status. The optional @c subsysName is used in reporting status changes
    //! as the subsystem name.
    SubsystemThread( gbxutilacfr::Tracer& tracer, gbxutilacfr::Status& status, const std::string& subsysName="SubsystemThread" );

    // from IceUtil::Thread
    //! This implementation calls walk(), catches all possible exceptions, prints out 
    //! errors and waits for someone to call stop().
    virtual void run();

    //! Implement this function in the derived class and put here all the stuff which your
    //! thread needs to do.
    virtual void walk()=0;

    //! Access to subsystem status.
    gbxutilacfr::SubStatus& subStatus() { return subStatus_; };

    //! Returns subsystem name assigned to this thread.
    std::string subsysName() const { return subStatus_.name(); };

private:
    gbxutilacfr::Tracer& tracer_;
    gbxutilacfr::SubStatus subStatus_;
};
//! A smart pointer to the SubsystemThread class.
typedef IceUtil::Handle<SubsystemThread> SubsystemThreadPtr;

}
} // end namespace

#endif
