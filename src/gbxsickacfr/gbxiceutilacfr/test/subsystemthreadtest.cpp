/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics 
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>

#include <IceUtil/Time.h>

#include <gbxsickacfr/gbxiceutilacfr/subsystemthread.h>
#include <gbxutilacfr/trivialtracer.h>
#include <gbxutilacfr/trivialstatus.h>

using namespace std;

class TestThread : public gbxiceutilacfr::SubsystemThread
{
public:    
    // it's safe to pass zero pointers
    TestThread( gbxutilacfr::Tracer& tracer, gbxutilacfr::Status& status ) : 
        SubsystemThread( tracer, status ) {};
    virtual void walk()
    {
        while ( !isStopping() ) {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(50));
        }
    };
};

class TestThreadWithThrow : public gbxiceutilacfr::SubsystemThread
{
public:
    // it's safe to pass zero pointers
    TestThreadWithThrow( gbxutilacfr::Tracer& tracer, gbxutilacfr::Status& status ) : 
        SubsystemThread( tracer, status ) {};
    virtual void walk()
    {
        throw "throwing from walk";
    };
};

class TestThreadWithTools : public gbxiceutilacfr::SubsystemThread
{
public:
    TestThreadWithTools( gbxutilacfr::Tracer& tracer, gbxutilacfr::Status& status ) :
        SubsystemThread( tracer, status, "MyName" )
    {
    };
    virtual void walk()
    {
        throw "throwing from walk";
    };
};

int main(int argc, char * argv[])
{
    gbxutilacfr::TrivialTracer tracer;
    gbxutilacfr::TrivialStatus status( tracer );

    cout<<"testing start() and stop()... ";
    {
        gbxiceutilacfr::Thread* t=0;
        try
        {
            t = new TestThread( tracer, status );
            t->start();
        }
        catch (...)
        {
            cout<<"failed"<<endl<<"should be able to create thread"<<endl;
            exit(EXIT_FAILURE);
        }
        IceUtil::ThreadControl tc = t->getThreadControl();
        t->stop();
        tc.join();
        // do not delete t! it's already self-destructed.
    }
    cout<<"ok"<<endl;

    cout<<"testing SubsystemThread() with exceptions... ";
    {
        gbxiceutilacfr::Thread* t=0;
        try
        {
            t = new TestThreadWithThrow( tracer, status );
            t->start();
        }
        catch (...)
        {
            cout<<"failed"<<endl<<"all exception should've been caught."<<endl;
            // ok
        }
        IceUtil::ThreadControl tc = t->getThreadControl();
        t->stop();
        tc.join();
        // do not delete t! it's already self-destructed.
    }
    cout<<"ok"<<endl;

    return EXIT_SUCCESS;
}
