/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://!gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXUTILACFR_TRACER_H
#define GBXUTILACFR_TRACER_H

#include <string>

namespace gbxutilacfr {

//! 
//! @brief Local and remote tracing.
//! 
//! Tracer is used to log human-readable trace statements, e.g. warnings, error messages, etc (not data).
//! 
//! A single Tracer object is meant to be shared by multiple threads, so the
//! implementation must be thread-safe.
//! 
//! When the tracing message is cheap to generate, simply call one of the tracing functions.
//! The routing to destinations will be performed internally.
//! 
//! @verbatim
//! std::string s = "something is wrong";
//! tracer->error( s );
//! @endverbatim
//! 
//! If the tracing message is a result of an expensive operation, you
//! many want to perform the tracing test yourself and then call the
//! tracing function. You may test verbosity for specific TraceType /
//! DestinationType combinations or use summary fields AnyTrace and
//! ToAny.
//! 
//! @verbatim
//! Tracer* tracer = context().tracer();
//! if ( tracer.verbosity( gbxutilacfr::Tracer::ErrorTrace, gbxutilacfr::Tracer::ToAny ) > 0 ) {
//!     std::string s = expensiveOperation();
//!     tracer.error( s );
//! }
//! @endverbatim
//! 
//! Enum gbxutilacfr::Tracer::TraceType defines types of traced
//! information. Enum gbxutilacfr::Tracer::DestinationType defines possible
//! tracer destinations are. Verbosity levels range from 0 (nothing) to
//! 10 (everything). The built-in defaults are as follows:
//! @verbatim
//!                 ToDisplay   ToNetwork   ToLog   ToFile
//! Info                1           0         0       0
//! Warning             1           0         0       0
//! Error              10           0         0       0
//! Debug               0           0         0       0
//! @endverbatim
//! 
//! @see Status
//!
class Tracer
{
public:
    virtual ~Tracer() {}; 

    //! Types of traced information
    enum TraceType {
        //! Information
        InfoTrace=0,
        //! Warning
        WarningTrace,
        //! Error
        ErrorTrace,
        //! Debug statement
        DebugTrace,
        //! Use this index to find out the maximum verbosity among all trace types to
        //! a particular destination.
        AnyTrace,
        //! Number of trace types
        NumberOfTraceTypes,
        //! Other 
        //! (not used by orcaice tracers, so it's verbosity level is not stored in
        //! our matrix, so it's safe to have it larger than NumberOfTraceTypes)
        OtherTrace
    };

    //! Types of destinations for traced information.
    enum DestinationType {
        //! Write to stardard display
        ToDisplay=0,
        //! Send over the network, details specific to Tracer implementation
        ToNetwork,
        //! Write to SysLog on Unix, EventLog on windows
        ToLog,
        //! Write to a file
        ToFile,
        //! Use this index to request the maximum verbosity of a particular type among 
        //! all destinations
        ToAny,
        //! Number of destination types
        NumberOfDestinationTypes
    };

    struct Config
    {
        //! array of verbosity levels
        int verbosity[NumberOfTraceTypes][NumberOfDestinationTypes];
        //! affects only the printout to stdout. Remote messages always have a timestamp.
        bool addTimestamp;
    };

    //! LOCAL INTERFACE
    
    //! Prints out verbatim to stdout. It is never routed over the network.
    //! @see info
    virtual void print( const std::string &message ) = 0;

    //! Routing is determined by InfoToXxx parameter.
    virtual void info( const std::string &message, int level=1 ) = 0;
    
    //! Routing is determined by WarningToXxx parameter.
    virtual void warning( const std::string &message, int level=1 ) = 0;
    
    //! Routing is determined by ErrorToXxx parameter.
    virtual void error( const std::string &message, int level=1 ) = 0;

    //! Routing is determined by DebugToXxx parameter.
    virtual void debug( const std::string &message, int level=1 ) = 0;

    //! Returns the verbosity level for traceType to destType. This test is performed 
    //! internally by all tracing functions, e.g. error(). You may want to call this 
    //! function yourself @e before calling error() if there is a significant overhead
    //! in forming the tracing string. See class documentation for an example of such
    //! usage.
    virtual int verbosity( TraceType traceType, DestinationType destType=ToAny ) const = 0;

    static std::string toString( Tracer::TraceType type )
    {
        switch ( type ) 
        {
        case Tracer::InfoTrace :
            return "info";
        case Tracer::WarningTrace :
            return "warn";
        case Tracer::ErrorTrace :
            return "error";
        case Tracer::DebugTrace :
            return "debug";
        default :
            return "other";
        }
    };

    static Tracer::TraceType toTraceType( const std::string& category )
    {
        if ( category==std::string("info") )
            return Tracer::InfoTrace;
        else if ( category==std::string("warn") )
            return Tracer::WarningTrace;
        else if ( category==std::string("error") )
            return Tracer::ErrorTrace;
        else if ( category==std::string("debug") )
            return Tracer::DebugTrace;
        else
            return Tracer::OtherTrace;
    };


    //! @b EXPERIMENTAL. Sets debug level for an individual subsystem.
    //! Only debug-to-display is supported.
    virtual void setLevel( const std::string &subsystem, int level=0 ) {};

    //! @b EXPERIMENTAL. Debug tracing for an individual subsystem.
    //! Only debug-to-display is supported.
    //! The maximum traceable level is set per-subsystem with setLevel().
    virtual void debug( const std::string &subsystem, const std::string &message, int level=1 ) 
    {
        debug( message, level );
    };
};

} //! namespace

#endif
