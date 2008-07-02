/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <sstream>
#include <gbxsmartbatteryacfr/exceptions.h>
#include "oceanserver.h"

namespace gbxsmartbatteryacfr {

OceanServer::OceanServer( const std::string   &port, 
                          gbxutilacfr::Tracer &tracer)
    : tracer_(tracer)
{
    reader_.reset(new gbxsmartbatteryacfr::OceanServerReader( port, tracer_ ));
}

void
OceanServer::read()
{
    try
    {
        gbxsmartbatteryacfr::OceanServerSystem data;
        reader_->read(data);
        gbxsmartbatteryacfr::updateWithNewData( data, data_ );
    }
    catch ( gbxsmartbatteryacfr::ParsingException &e )
    {
        stringstream ss;
        ss << "OceanServer: " << __func__ << ": Caught a parsing exception: "
           << e.what() << endl
           << "This can happen sometimes. Will continue regardless.";
        tracer_.info( ss.str() );
    }
}

const gbxsmartbatteryacfr::OceanServerSystem&
OceanServer::getData() const
{
    return data_;
}

}
