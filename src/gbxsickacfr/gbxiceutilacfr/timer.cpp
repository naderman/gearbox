/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <gbxsickacfr/gbxiceutilacfr/timer.h>

namespace gbxiceutilacfr {

Timer::Timer() :
    time_( IceUtil::Time::now() )
{
}

Timer::Timer( const IceUtil::Time& elapsedTime ) :
    time_( IceUtil::Time::now()-elapsedTime )
{
}

void Timer::restart()
{
    time_ = IceUtil::Time::now();
}

IceUtil::Time Timer::elapsed() const
{
    return IceUtil::Time::now() - time_;
}

double Timer::elapsedMs() const
{
    return ( (IceUtil::Time::now() - time_).toMilliSecondsDouble() );
}

double Timer::elapsedSec() const
{
    return ( (IceUtil::Time::now() - time_).toSecondsDouble() );
}

} // namespace
