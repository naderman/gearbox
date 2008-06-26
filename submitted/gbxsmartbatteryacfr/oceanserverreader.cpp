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

#include "oceanserverreader.h"

using namespace std;

namespace gbxsmartbatteryacfr {
    
static const int BAUDRATE = 19200;
static const int TIMEOUT_SEC = 2;
 
OceanServerReader::OceanServerReader( const string        &device,
                                      gbxutilacfr::Tracer &tracer )
    : serial_( device, BAUDRATE, gbxserialacfr::Serial::Timeout(TIMEOUT_SEC,0) ),
      tracer_(tracer),
      parser_(tracer),
      firstTime_(true)
{
    // some menu entries from the OceanServer system - used to recognize whether
    // we are connected to the right device
    oceanServerStrings_.push_back(" S - Setup Controller");
    oceanServerStrings_.push_back(" B - Battery Status");
    oceanServerStrings_.push_back(" X - Host HEX");
    oceanServerStrings_.push_back(" H - Help");
    oceanServerStrings_.push_back(" www.ocean-server.com");
    
    checkConnection();

    // send the command to start reading data
    serial_.flush();
    
    const char startReading = 'X';
    
    serial_.write(&startReading, 1);
}

bool
OceanServerReader::isOceanServerSystem( const char* oceanServerString )
{
    for (unsigned int i=0; i<oceanServerStrings_.size(); i++)
    {
        // if the first 8 characters agree we are pretty sure we have an OceanServerSystem
        if (strncmp(oceanServerStrings_[i].c_str(),oceanServerString,8)==0) return true;
    }
    return false;
}

void
OceanServerReader::checkConnection()
{   
    tracer_.info( "OceanServerReader: Checking connection to serial port" );

    // a blank will get us into menu mode
    const char menuMode = ' ';
    serial_.write(&menuMode, 1);
    
    // if we were in battery reading mode before, we might have to skip quite 
    // a few lines until we get something from the menu
    const int maxTries = 40;
    int numTries=0;
    
    // for tracer output
    stringstream ss;

    while(true)
    {
        ss.str(""); ss << "OceanServerReader: Trying to read from serial port with timeout of " << TIMEOUT_SEC << "s" << endl;
        tracer_.info( ss.str() );
        
        string serialData;        
        int ret = serial_.readLine( serialData );
        if (ret<0)  {
          throw HardwareReadingException( ERROR_INFO, "Connected to the wrong serial port. Timed out while trying to read a line.");
        }
        if ( isOceanServerSystem(serialData.c_str()) ) {
            tracer_.info( "Oceanserverreader.cpp: We are connected to an Oceanserver system. Good." );
            break;
        }
        numTries++;
        ss.str(""); ss << "OceanServerReader: Trying to find out whether this is an oceanserver system. Attempt number " << numTries << "/" << maxTries << ".";
        tracer_.info( ss.str() );
        if (numTries>=maxTries) {
            throw HardwareReadingException( ERROR_INFO, "Connected to the wrong serial port. Didn't recognize any of the strings.");
        }
    }
}

OceanServerReader::~OceanServerReader()
{
}    

void 
OceanServerReader::tryToReadLineFromSerialPort( std::string &serialData )
{
    const int maxTries=5;
    int numTries=0;
    
    while(true)
    {
        int ret = serial_.readLine( serialData );

        if (ret>0) break;
        
        numTries++;
        if (numTries>=maxTries) {
            stringstream ss;
            ss << "Can't read data from serial port. Timed out and/or empty strings " << maxTries << " times in a row.";
            throw HardwareReadingException( ERROR_INFO,  ss.str().c_str() );
        }
    }
}

void 
OceanServerReader::read( OceanServerSystem &system )
{
    string serialData;
    vector<string> stringList;

    if (firstTime_) 
    {
        // (1) Wait until we got the beginning of the record
        while(true)
        {
            tryToReadLineFromSerialPort( serialData );
            if (parser_.atBeginningOfRecord( serialData.c_str() )) break;
        }
        
        tracer_.debug( "OceanServerReader: Beginning of a new record", 5 );
        
        // (2) Add the first line to the stringlist
        stringList.push_back( serialData );
    }
    else
    {
        
        tracer_.debug( "OceanServerReader: We already have the first line from the previous record", 5 );
        stringList.push_back( beginningRecordLine_ );
    }
    
    try {
        
        // (3) Read the rest of the record line-by-line
        while(true)
        {        
            tryToReadLineFromSerialPort( serialData );
            if ( parser_.atEndOfRecord( serialData.c_str() ) ) 
            {   
                tracer_.debug( "OceanServerReader: End of record", 5 );
                parser_.parse( stringList, system );
                break; 
            }
            stringList.push_back(serialData);
        }
        
        // (4) Save the last line for next time, it's the S-record
        //     Otherwise we'd miss a record
        beginningRecordLine_ = serialData;
        if (firstTime_) {
            firstTime_ = false;
        }
    } 
    catch (ParsingException &e)
    {
        stringstream ss;
        ss << "OceanServerReader: Caught ParsingException: " << e.what() << ". ";
        ss << "It's not critical, we are trying to find the beginning of a new record.";
        tracer_.warning( ss.str() );
        firstTime_ = true;
        
        // we have to rethrow, so that the caller knows that it may receive a corrupt record
        throw;
    }
}

}
         
