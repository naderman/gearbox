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
#include <sstream>

#include "subsystemthread.h"

using namespace gbxsickacfr::gbxiceutilacfr;
using namespace std;


SubsystemThread::SubsystemThread( gbxutilacfr::Tracer& tracer, gbxutilacfr::Status& status, const std::string& subsysName ) :
    tracer_(tracer),
    subStatus_( status, subsysName )
{
}

void 
SubsystemThread::run()
{
    stringstream ss;
    try
    {
        walk();
    }
    catch ( const IceUtil::Exception &e )
    {
        ss << "SubsystemThread::run() "<<subsysName()<<": Caught unexpected exception: " << e;
    }
    catch ( const std::exception &e )
    {
        ss << "SubsystemThread::run() "<<subsysName()<<": Caught unexpected exception: " << e.what();
    }
    catch ( const std::string &e )
    {
        ss << "SubsystemThread::run() "<<subsysName()<<": Caught unexpected string: " << e;
    }
    catch ( const char *e )
    {
        ss << "SubsystemThread::run() "<<subsysName()<<": Caught unexpected char *: " << e;
    }
    catch ( ... )
    {
        ss << "SubsystemThread::run() "<<subsysName()<<": Caught unexpected unknown exception.";
    }

    // report status fault if there was an exception and we are not stopping
    if ( !ss.str().empty() && !isStopping() ) 
    {
        tracer_.error( ss.str() );
        subStatus().fault( ss.str() );
    }
    else {
        tracer_.debug( subsysName()+": dropping out from run() ", 4 );
    }

    // wait for somebody to realize that we are quitting and tell us to stop.
    waitForStop();
}
