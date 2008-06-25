/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBX_SMARTBATTERY_ACFR_EXCEPTIONS_H
#define GBX_SMARTBATTERY_ACFR_EXCEPTIONS_H

#include <exception>
#include <string>

namespace gbxsmartbatteryacfr {
    
//!
//! Exceptions for gbxsmartbatteryacfr
//!
class Exception : public std::exception
{
public:

    Exception(const char *message)
        : message_(message) {}
    Exception(const std::string &message)
        : message_(message) {}

    virtual ~Exception() throw() {}

    virtual const char* what() const throw() { return message_.c_str(); }

protected:

    std::string  message_;
};

class HardwareReadingException : public Exception
{  
    public:
        HardwareReadingException( const char * message )
            : Exception( message ) {}     
};

class ParsingException : public Exception
{  
    public:
        ParsingException( const char * message )
            : Exception( message ) {}    
};

}

#endif

