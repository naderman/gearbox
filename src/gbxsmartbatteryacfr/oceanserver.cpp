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
    
static const int MAX_EXCEPTIONS_ROW = 10;

OceanServer::OceanServer( const std::string   &port, 
                          gbxutilacfr::Tracer &tracer)
    : tracer_(tracer),
      exceptionCounter_(0)
{
    reader_.reset(new gbxsmartbatteryacfr::OceanServerReader( port, tracer_ ));
}

const gbxsmartbatteryacfr::OceanServerSystem&
OceanServer::getData()
{
    try
    {
        // read new data, this may throw
        gbxsmartbatteryacfr::OceanServerSystem data;
        reader_->read(data);
        
        // if successful, reset counter
        exceptionCounter_ = 0;
        
        // update internal (full) record
        gbxsmartbatteryacfr::updateWithNewData( data, data_ );
    }
    catch ( gbxsmartbatteryacfr::ParsingException &e )
    {
        stringstream ss;
        ss << "OceanServer: " << __func__ << ": Caught a parsing exception: "
           << e.what() << endl
           << "This can happen sometimes. Will continue regardless.";
        tracer_.debug( ss.str(), 3 );
        
        exceptionCounter_++;
        if (exceptionCounter_ >= MAX_EXCEPTIONS_ROW) 
        {
            ss.str(""); 
            ss << "OceanServer: " << __func__ << ": Caught " << MAX_EXCEPTIONS_ROW 
               << " ParsingExceptions in a row. Something must be wrong";
            throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
        }
    }
    
    // return updated internal storage
    // if there was an exception on read, we just return the previous record
    return data_;
}

}
