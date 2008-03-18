/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include "exceptions.h"
#include <iostream>
#include <cstring>

using namespace std;

namespace gbxsickacfr {
namespace gbxutilacfr {

Exception::Exception( const char *file, const char *line, const char *message )
{
    setMsg( file, line, message );
}

Exception::Exception( const char *file, const char *line, const std::string &message )
{
    setMsg( file, line, message.c_str() );
}

Exception::~Exception() throw()
{
}

const char *
Exception::basename( const char *s )
{
#ifndef WIN32
    return strrchr( s, '/' )+1;
#else
    return strrchr( s, '\\' )+1;
#endif
};

void
Exception::setMsg( const char *file, const char *line, const char *message )
{
    std::string msgString(message);
    message_ =  "\n *** ERROR(";
    // not to confuse our local basename() with gbxutilacfr::basename()
    message_ += this->basename(file);
    message_ += ":";
    message_ += line;
    message_ += +"): " + msgString + "\n";
}

}
} // namespace
