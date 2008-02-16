/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include "trivialtracer.h"
#include <iostream>

using namespace std;

namespace gbxsickacfr {
namespace gbxutilacfr {

TrivialTracer::TrivialTracer( bool debug, bool info, bool warn, bool error ) :
    debug_(debug),
    info_(info),
    warn_(warn),
    error_(error)
{
}

void
TrivialTracer::print( const std::string &message )
{
    cout<<message<<endl;
}

void
TrivialTracer::info( const std::string &message, int level )
{
    if ( info_ )
        cout << "info: " << message << endl;
}

void
TrivialTracer::warning( const std::string &message, int level )
{
    if ( warn_ )
        cout << "warn: " << message << endl;
}
    
void
TrivialTracer::error( const std::string &message, int level )
{
    if ( error_ )
        cout << "error: " << message << endl;
}

void
TrivialTracer::debug( const std::string &message, int level )
{
    if ( debug_ )
        cout << "debug: " << message << endl;
}

int 
TrivialTracer::verbosity( TraceType traceType, DestinationType destType ) const
{
    return 10;
}

}
}

