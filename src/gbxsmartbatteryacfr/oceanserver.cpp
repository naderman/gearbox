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

static const int MAX_EXCEPTIONS_BEFORE_RESET = 10;
static const int MAX_EXCEPTIONS_BEFORE_CRITICAL = 20;

OceanServer::OceanServer( const std::string   &port, 
                          gbxutilacfr::Tracer &tracer)
    : tracer_(tracer),
      exceptionCounter_(0),
      exceptionString_("")
{
    reader_.reset(new gbxsmartbatteryacfr::OceanServerReader( port, tracer_ ));
}

const gbxsmartbatteryacfr::OceanServerSystem&
OceanServer::getData()
{
    gbxsmartbatteryacfr::OceanServerSystem data;

    try
    {
        // read new data, this may throw
        reader_->read(data);
        
        // successful read: reset counter and string
        exceptionCounter_ = 0;
        exceptionString_ = "";
        
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
        stringstream ssEx;
        ssEx << e.what() << endl;
        for (unsigned int i=0; i<data.rawRecord().size(); i++)
            ssEx << data.rawRecord()[i] << endl;
        ssEx << endl;
        exceptionString_ = exceptionString_ + ssEx.str();

        if (exceptionCounter_ >= MAX_EXCEPTIONS_BEFORE_RESET)
        {
            stringstream ss;
            ss << "OceanServer: " << __func__ << ": Caught " << MAX_EXCEPTIONS_BEFORE_RESET
               << " ParsingExceptions in a row. Resetting the reader now...";
            tracer_.warning( ss.str() );
            reader_->reset();
        }
        
        if (exceptionCounter_ >= MAX_EXCEPTIONS_BEFORE_CRITICAL) 
        {
            ss.str(""); 
            ss << "OceanServer: " << __func__ << ": Caught " << MAX_EXCEPTIONS_BEFORE_CRITICAL 
               << " ParsingExceptions in a row. Something must be wrong. Here's the history: " << endl
               << exceptionString_;
            throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
        }
    }
    
    // return updated internal storage
    // if there was an exception on read which does not get re-thrown above,
    // the internal storage will contain the previous record
    return data_;
}

}
