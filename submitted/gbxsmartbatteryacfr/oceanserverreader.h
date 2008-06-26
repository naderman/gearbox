/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBX_OCEANSERVER_READER_H
#define GBX_OCEANSERVER_READER_H

#include <gbxserialacfr/serial.h>
#include <gbxutilacfr/tracer.h>

#include <gbxsmartbatteryacfr/oceanserverparser.h>


namespace gbxsmartbatteryacfr
{  
    
//!
//! Class to read data from the oceanserver battery system: 
//! (1) connects to the serial port 
//! (2) reads from the serial port and parses data
//!
//! @author Tobias Kaupp
//!
class OceanServerReader
{
public:
    
    //! May throw HardwareReadingException
    OceanServerReader( const std::string   &device,
                       gbxutilacfr::Tracer &tracer );

    ~OceanServerReader() {};
    
    //! May throw HardwareReadingException
    void read( OceanServerSystem &system );

private:   
    
    bool isOceanServerSystem( const char* oceanServerString );
    std::vector<std::string> oceanServerStrings_;
    
    gbxserialacfr::Serial serial_;
    gbxutilacfr::Tracer& tracer_;
    gbxsmartbatteryacfr::OceanServerParser parser_;
    
    void checkConnection();
    void tryToReadLineFromSerialPort( std::string &serialData );
    
    std::string beginningRecordLine_;
    bool firstTime_;
};

} // namespace

#endif
