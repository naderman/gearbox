/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <IceUtil/Time.h>
#include <gbxsickacfr/gbxutilacfr/exceptions.h>

#include "thread.h"

namespace gbxsickacfr {
namespace gbxiceutilacfr {

Thread::Thread() : 
    isStopping_(false) 
{
}

bool 
Thread::isStarted()
{
    // must use this mutex from IceUtil::Thread
    IceUtil::Mutex::Lock lock(_stateMutex);
    return _started;
}

void 
Thread::stop()
{
    // using the mutex from IceUtil::Thread for convenience
    IceUtil::Mutex::Lock lock(_stateMutex);
    isStopping_ = true;
}

bool 
Thread::isStopping()
{
    // using the mutex from IceUtil::Thread for convenience
    IceUtil::Mutex::Lock lock(_stateMutex);
    return isStopping_;
}

void 
Thread::waitForStop()
{
    while ( !isStopping() ) {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(10));
    }
}

void stopAndJoin( gbxiceutilacfr::Thread* thread )
{
    if ( thread ) {
        // get the control object first
        IceUtil::ThreadControl tc = thread->getThreadControl();
        
        // Tell the thread to stop
        thread->stop();
    
        // Then wait for it
        tc.join();
    }
}

void stopAndJoin( const gbxiceutilacfr::ThreadPtr& thread )
{
    if ( thread ) {
        // get the control object first
        IceUtil::ThreadControl tc = thread->getThreadControl();
        
        // Tell the thread to stop
        thread->stop();
    
        // Then wait for it
        tc.join();        
    }
}

void checkedSleep( const gbxiceutilacfr::ThreadPtr& thread, IceUtil::Time duration, int checkIntervalMs  )
{
    IceUtil::Time wakeupTime = IceUtil::Time::now() + duration;
    IceUtil::Time checkInterval = IceUtil::Time::milliSeconds( checkIntervalMs );

    while ( !thread->isStopping() && IceUtil::Time::now() < wakeupTime )
    {
        IceUtil::ThreadControl::sleep( checkInterval );
    }
}

// void gbxiceutilacfr::stopAndJoin( const std::vector<gbxiceutilacfr::Thread *> &threads )
// {
//     std::vector<IceUtil::ThreadControl*> tc;
//     for ( uint i=0; i < threads.size(); i++ )
//     {
//         if ( !threads[i] )
//         {
//             tc[i] = NULL;
//             continue;
//         }

//         tc[i] = new IceUtil::ThreadControl( threads[i]->getThreadControl() );
//         threads[i]->stop();
//     }

//     for ( uint i=0; i < threads.size(); i++ )
//     {
//         if ( tc[i] == NULL )
//             continue;

//         tc[i]->join();

//         delete tc[i];
//     }
// }

}
} // namespace
