/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXICEUTILACFR_THREAD_H
#define GBXICEUTILACFR_THREAD_H

#include <IceUtil/Thread.h>

namespace gbxsickacfr {
namespace gbxiceutilacfr {

/*!
@brief A minor extention of the IceUtil::Thread class.

Adds an option to stop a thread with @ref stop(). Requires that the thread periodically checks
whether it has to stop by calling isStopping(); Since @ref stop() is public, it can be called
from inside or outside of the derived class.

To use this class, simply overload the virtual IceUtil::Thread::run() function.
@verbatim
void MyThread::run()
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

The implicit state machine of this class has 4 states {Starting, Running, Stopping, Stopped}. Events and the corresponding state transitions are listed below. Note that the final state may only be accessed when using smart pointer gbxiceutilacfr::ThreadPtr (otherwise the thread self-destructs).

@verbatim
EVENT      : Constructor IceUtil::Thread::Thread()
TRANSITION : Starting    
INTERNAL   : isStopping()=false, isAlive()=false, isStarted()=false

EVENT      : IceUtil::Thread::start()
TRANSITION : Starting -> Running     
INTERNAL   : isStopping()=false, isAlive()=true, isStarted()=true

EVENT      : gbxiceutilacfr::Thread::stop()
TRANSITION : Running -> Stopping    
INTERNAL   : isStopping()=true, isAlive()=true, isStarted()=true

EVENT      : termination of run() function.
TRANSITION : Stopping -> Stopped     
INTERNAL   : isStopping()=true, isAlive()=false, isStarted()=true
@endverbatim

Caveats: 
- Make sure you catch all exception which can possibly be raised inside IceUtil::Thread::run.
Otherwise, you'll see "uncaught exception" printed out and the component will hang.
- gbxiceutilacfr::Threads self-destruct (ie call their own destructor) when gbxiceutilacfr::Thread::run returns, unless you hold onto an gbxiceutilacfr::ThreadPtr which points to it. So never call @c delete on the pointer to a thread, doing so will result in segmentation fault.

@see SafeThread
 */
class Thread : public IceUtil::Thread
{
public:

    Thread();

    //! Lets the thread know that it's time to stop. Thread-safe, so it can be called
    //! from inside or outside this thread.
    void stop();

    //! Returns TRUE if the thread is in Started state, FALSE otherwise.
    bool isStarted();

    //! Returns TRUE if the thread is in Stopping state, FALSE otherwise.
    bool isStopping();

    //! @b Depricated function! Use isStopping() instead (note that it returns the opposite).
    //! 
    //! Returns FALSE if thread was told to stop, TRUE otherwise.
    bool isActive() { return !isStopping(); };

protected:

    //! Wait for someone from the outside to call @ref stop.
    //! It may be necessary to call this function before exitting from IceUtil::Thread::run after
    //! catching an exception. If we just exit from run() and someone calls our @ref stop()
    //!  function afterwards there's a possibility of lock up.
    void waitForStop();

private:
    bool isStopping_;
};
//! A smart pointer to the thread class.
typedef IceUtil::Handle<gbxiceutilacfr::Thread> ThreadPtr;

//! A convenience function which first stops the @p thread and then waits for it to
//! terminate. If the pointer is NULL, this function quietly returns.
void stopAndJoin( gbxiceutilacfr::Thread* thread );

//! A convenience function which first stops the @p thread and then waits for it to
//! terminate. If the smart pointer is 0, this function quietly returns.
void stopAndJoin( const gbxiceutilacfr::ThreadPtr& thread );

//! Sleeps for @ref duration waking up every @ref checkIntervalMs [ms] to check if the @c thread 
//! was told to stop. This implementation is very simple so the error in total sleep duration 
//! can be as large as checkIntervalMs. In particular, if @ref duration is shorter than @ref checkIntervalMs,
//! this function will sleep for @ref checkIntervalMs.
void checkedSleep( const gbxiceutilacfr::ThreadPtr& thread, IceUtil::Time duration, int checkIntervalMs=250 );

}
} // end namespace

#endif
