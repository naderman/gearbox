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

#include "safethread.h"

using namespace gbxsickacfr::gbxiceutilacfr;
using namespace std;


SafeThread::SafeThread( gbxsickacfr::gbxutilacfr::Tracer& tracer ) :
    tracer_(tracer)
{
}

void 
SafeThread::run()
{
    stringstream ss;
    try
    {
        walk();
    }
    catch ( const IceUtil::Exception &e ) {
        ss << "SafeThread::run(): Caught unexpected exception: " << e;
    }
    catch ( const std::exception &e ) {
        ss << "SafeThread::run(): Caught unexpected exception: " << e.what();
    }
    catch ( const std::string &e ) {
        ss << "SafeThread::run(): Caught unexpected string: " << e;
    }
    catch ( const char *e ) {
        ss << "SafeThread::run(): Caught unexpected char *: " << e;
    }
    catch ( ... ) {
        ss << "SafeThread::run(): Caught unexpected unknown exception.";
    }

    // only if there were exceptions
    if ( !ss.str().empty() )  {
        tracer_.error( ss.str() );
    }
    else {
        tracer_.debug( "dropping out from run()", 4 );
    }

    // wait for the component to realize that we are quitting and tell us to stop.
    waitForStop();
}
