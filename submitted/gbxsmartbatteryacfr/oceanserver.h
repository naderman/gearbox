/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBX_OCEANSERVER_H
#define GBX_OCEANSERVER_H

#include <memory>
#include <gbxutilacfr/tracer.h>
#include <gbxsmartbatteryacfr/oceanserverreader.h>

using namespace std;

namespace gbxsmartbatteryacfr {

//! Class for reading data from an OceanServer battery system.
//! Wraps up all the logic required to read from the system and
//! maintain some incremental internal data storage.
//! Also handles all ParsingExceptions.
//!
//! @author Tobias Kaupp
//!
class OceanServer
{    
public:
    
    OceanServer( const std::string      &port,
                 gbxutilacfr::Tracer    &tracer);
    
    //! Reads data from OceanServer, incrementally updates internal storage    
    //! Returns a reference to the internal storage
    //! May throw gbxutilacfr::Exception
    const gbxsmartbatteryacfr::OceanServerSystem& getData();
    
private:
    
    gbxsmartbatteryacfr::OceanServerSystem data_;
    gbxutilacfr::Tracer& tracer_;
    auto_ptr<gbxsmartbatteryacfr::OceanServerReader> reader_;
    
    int exceptionCounter_;
    
};

} //namespace

#endif



