/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef SMARTBATTERY_PARSING_H
#define SMARTBATTERY_PARSING_H

#include <string>
#include <vector>
#include <map>

#include <gbxutilacfr/tracer.h>

namespace gbxsmartbatteryacfr
{
    
//! 
//! Functions to parse data using the smart battery data standard
//! and some helper functions
//!
//! @author Tobias Kaupp
//!
    
// Reading data
void readSingleByte( const std::string &str, 
                     std::vector<bool> &flags );
double readTemperature( const std::string &str );
double readCurrent( const std::string &str );
double readVoltage( const std::string &str );
int readNumBatteries( const std::string &str );
int readPercentWord( const std::string &str );
int readPercentByte( const std::string &str );
int readMinutes( const std::string &str );
int readCapacity( const std::string &str );
uint16_t read16Flags( const std::string &str );
int readCount( const std::string &str );
int readNumber( const std::string &str );
bool readBool( const std::string &str );
int readRate( const std::string &str );

// Other helper functions
bool isChecksumValid( const std::string &data, 
                      const std::string &checksumStr );
                      
void splitIntoFields( const std::string        &str, 
                      std::vector<std::string> &fields, 
                      const std::string        &delimiter);  
                     
void toKeyValuePairs( const std::vector<std::string>    &fields,
                      std::map<std::string,std::string> &pairs,
                      gbxutilacfr::Tracer               &tracer );

}

#endif
