/*
 * Orca-Robotics Project: Components for robotics 
             http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXUTILACFR_TRIVIAL_TRACER_H
#define GBXUTILACFR_TRIVIAL_TRACER_H

#include <gbxsickacfr/gbxutilacfr/tracer.h>

namespace gbxsickacfr {
namespace gbxutilacfr {

class TrivialTracer : public Tracer
{
public:
    TrivialTracer( bool debug=false, bool info=true, bool warn=true, bool error=true );

    virtual void print( const std::string &message );
    virtual void info( const std::string &message, int level=1 );
    virtual void warning( const std::string &message, int level=1 );
    virtual void error( const std::string &message, int level=1 );
    virtual void debug( const std::string &message, int level=1 );
    virtual int verbosity( TraceType traceType, DestinationType destType ) const;

private:
    bool debug_;
    bool info_;
    bool warn_;
    bool error_;
};

}
} // namespace

#endif
