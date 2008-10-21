/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXUTILACFR_TRIVIAL_TRACER_H
#define GBXUTILACFR_TRIVIAL_TRACER_H

#include <gbxutilacfr/tracer.h>

namespace gbxutilacfr {

//!
//! @brief A simple implementation of the tracer API which prints to cout.
//!
//! @see Tracer
//!
class TrivialTracer : public Tracer
{
public:

    //!
    //! Integers specify the tracing level: we'll print out all level at or below this.
    //!
    TrivialTracer( int debug=0,
                   int info=9,
                   int warn=9,
                   int error=9 );

    virtual void print( const std::string &message );
    virtual void info( const std::string &message, int level=1 );
    virtual void warning( const std::string &message, int level=1 );
    virtual void error( const std::string &message, int level=1 );
    virtual void debug( const std::string &message, int level=1 );
    virtual int verbosity( TraceType traceType, DestinationType destType=Tracer::ToAny ) const;

private:

    int traceLevels_[Tracer::NumberOfTraceTypes];
};

} // namespace

#endif
