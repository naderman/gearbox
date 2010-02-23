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

namespace gbxutilacfr {

Exception::Exception( const char *file, const char *line, const std::string &message )
    : message_(toMessageString(file,line,message))
{}

Exception::~Exception() throw()
{
}

const char *
Exception::basename( const char *s ) const
{
#ifndef WIN32
    char *slashPos = strrchr( s, '/' );
#else
    char *slashPos = strrchr( s, '\\' );
#endif
    if ( slashPos == NULL )
        return s; // no slash found
    else
        return slashPos+1;
};

std::string
Exception::toMessageString( const char *file, const char *line, const std::string &message ) const
{
    std::string msg = "\n *** ERROR(";
    // not to confuse our local basename() with gbxutilacfr::basename()
    msg += this->basename(file);
    msg += ":";
    msg += line;
    msg += "): " + message;
    return msg;
}

} // namespace
