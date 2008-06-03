/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Duncan Mercer, Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <sstream>
#include <gbxsickacfr/gbxutilacfr/gbxutilacfr.h>
#include <gbxgarminacfr/gbxgpsutilacfr/nmea.h>
#include <cstdlib>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "driver.h"

using namespace std;
using namespace gbxgarminacfr;

///////////////////////////////////////

namespace {

// Get the useful bits from a GGA message
GenericData* extractGgaData( gbxgpsutilacfr::NmeaMessage& msg, int timeSec, int timeUsec )
{
    GgaData* data = new GgaData;

    data->timeStampSec = timeSec;
    data->timeStampUsec = timeUsec;

    //Names for the tokens in the GGA message
    enum GgaTokens{MsgType=0,UTC,Lat,LatDir,Lon,LonDir,FixType,
                   NSatsUsed,HDOP,Hgt,M1,GeoidHgt,M2,DiffAge,DiffId};

    // fix type
    switch (msg.getDataToken(FixType)[0])
    {
    case '0': 
        data->fixType = Invalid;
        // NOTE: not processing the rest!
        return data;
    case '1': 
        data->fixType = Autonomous;
        break;
    case '2': 
        data->fixType = Differential;
        break;
    }
        
    //UTC time 
    sscanf(msg.getDataToken(UTC).c_str(),"%02d%02d%lf",
           &data->utcTimeHrs, &data->utcTimeMin, &data->utcTimeSec );
    //position
    int deg;
    double min;
    double dir;
    
    //latitude
    sscanf(msg.getDataToken(Lat).c_str(),"%02d%lf",&deg,&min);
    dir = (*msg.getDataToken(LatDir).c_str()=='N') ? 1.0 : -1.0;
    data->latitude=dir*(deg+(min/60.0));
    //longitude
    sscanf(msg.getDataToken(Lon).c_str(),"%03d%lf",&deg,&min);
    dir = (*msg.getDataToken(LonDir).c_str()=='E') ? 1.0 : -1.0;
    data->longitude=dir*(deg+(min/60.0));
    
    //number of satellites in use
    data->satellites = atoi(msg.getDataToken(NSatsUsed).c_str());
    
    //altitude
    data->altitude=atof(msg.getDataToken(Hgt).c_str());
    
    //geoidal Separation
    data->geoidalSeparation=atof(msg.getDataToken(GeoidHgt).c_str());

    return data;
}

// VTG provides velocity and heading information
GenericData* extractVtgData( gbxgpsutilacfr::NmeaMessage& msg, int timeSec, int timeUsec )
{
    VtgData* data = new VtgData;

    data->timeStampSec = timeSec;
    data->timeStampUsec = timeUsec;

    //Names for the VTG message items
    enum VtgTokens{MsgType=0,HeadingTrue,T,HeadingMag,M,SpeedKnots,
                   N,SpeedKPH,K,ModeInd};

    //Check for an empty string. Means that we are not moving
    //When the message has empty fields tokeniser skips so we get the next field inline.
    if(msg.getDataToken(HeadingTrue)[0] == 'T' ) {
        data->headingTrue=0.0;
        data->headingMagnetic=0.0;
        data->speed=0.0;
        data->climbRate=0.0;
        // NOTE: not processing the rest!
        return data;
    }

    // true heading
    double headingRad = DEG2RAD(atof(msg.getDataToken(HeadingTrue).c_str()));
    NORMALISE_ANGLE( headingRad );
    data->headingTrue=headingRad;

    // magnetic heading
    headingRad = DEG2RAD(atof(msg.getDataToken(HeadingMag).c_str()));
    NORMALISE_ANGLE( headingRad );
    data->headingMagnetic=headingRad;

    //speed - converted to m/s
    data->speed=atof(msg.getDataToken(SpeedKPH).c_str());
    data->speed*=(1000/3600.0);

    //set to zero
    data->climbRate=0.0;

    return data;
}

// RME message. This one is garmin specific... Give position error estimates
// See doc.dox for a discussion of the position errors as reported here. 
// Essentially the EPE reported by the garmin is a 1 sigma error (RMS) or a
// 68% confidence bounds.
GenericData* extractRmeData( gbxgpsutilacfr::NmeaMessage& msg, int timeSec, int timeUsec )
{
    RmeData* data = new RmeData;

    data->timeStampSec = timeSec;
    data->timeStampUsec = timeUsec;

    //Names for the RME message items
    enum RmeTokens{MsgType=0,HError,M1,VError,M2,EPE,M3};
    
    data->horizontalPositionError = atof(msg.getDataToken(HError).c_str());
    data->verticalPositionError = atof(msg.getDataToken(VError).c_str());

    return data;
}

}

///////////////////////////////////////

bool
Config::isValid() const
{
    if ( device.empty() ) return false;

    return true;
}

std::string
Config::toString() const
{
    std::stringstream ss;
    ss << "Garmin driver config: " <<
          "\tdevice="<<device <<
          "\twill read sentences: GPGGA="<<readGga<<" GPVTG="<<readVtg<<" PGRME="<<readRme;
    return ss.str();
}

/////////////////////

Driver::Driver( const Config &config, 
            gbxsickacfr::gbxutilacfr::Tracer &tracer,
            gbxsickacfr::gbxutilacfr::Status &status ) :
    config_(config),
    tracer_(tracer),
    status_(status)
{
    if ( !config_.isValid() )
    {
        stringstream ss;
        ss << "Invalid config: " << config_.toString();
        throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }

    stringstream ssDebug;
    ssDebug << "Connecting to GPS on serial port " << config_.device;
    tracer_.debug( ssDebug.str() );

    // BAUD rate is dictated by the NMEA standard.
    int baud = 4800;

    // the first 5 initialization messages come in 1 sec. Therefore, 2 secs should be conservative.
    serial_.reset( new gbxserialacfr::Serial( config_.device, baud, gbxserialacfr::Serial::Timeout(2,0) ) );

    init();
}

Driver::~Driver()
{
    disableDevice();
}

void
Driver::init()
{ 
    try {
        enableDevice();
        //TODO Need to check here that we have been successful.
    }
    catch ( const gbxserialacfr::SerialException &e )
    {
        stringstream ss;
        ss << "Driver: Caught SerialException: " << e.what();
        tracer_.error( ss.str() );
        throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }
}

void
Driver::enableDevice()
{
    //Create the messages that we are going to send and add the checksums
    //Note that the checksum field is filled with 'x's before we start
    gbxgpsutilacfr::NmeaMessage DisableAllMsg("$PGRMO,,2*xx\r\n",gbxgpsutilacfr::AddChecksum);
    gbxgpsutilacfr::NmeaMessage Start_GGA_Msg("$PGRMO,GPGGA,1*xx\r\n",gbxgpsutilacfr::AddChecksum);
    gbxgpsutilacfr::NmeaMessage Start_VTG_Msg("$PGRMO,GPVTG,1*xx\r\n",gbxgpsutilacfr::AddChecksum);
    gbxgpsutilacfr::NmeaMessage Start_RME_Msg("$PGRMO,PGRME,1*xx\r\n",gbxgpsutilacfr::AddChecksum);

    tracer_.info("Configure Garmin GPS device");


    //First disables all output messages then enable selected ones only.
    serial_->writeString(DisableAllMsg.sentence());
    sleep(1);
    
    serial_->writeString(Start_GGA_Msg.sentence());
    serial_->writeString(Start_VTG_Msg.sentence());
    serial_->writeString(Start_RME_Msg.sentence());
    sleep(1);
}

void
Driver::disableDevice()
{
    //Simply send the no messages command!
    gbxgpsutilacfr::NmeaMessage DisableAllMsg("$PGRMO,,2*xx\r\n",gbxgpsutilacfr::AddChecksum);
    serial_->writeString(DisableAllMsg.sentence());
}

std::auto_ptr<GenericData>  
Driver::read()
{  
    std::auto_ptr<GenericData> genericData;

    gbxgpsutilacfr::NmeaMessage nmeaMessage;
    // Make sure that we clear our internal data structures
    // alexm: is this necessary? should NmeaMessage do it for itself?
    memset((void*)(&nmeaMessage) , 0 , sizeof(nmeaMessage));

    int nmeaExceptionCount = 0;
    int nmeaFailChecksumCount = 0;

    while ( true )
    {
        string serialData;
    
        //
        // This will block up to the timeout
        //
        tracer_.debug( "Driver::read(): calling serial_->readLine()", 10 );
        int ret = serial_->readLine(serialData);
    
        // get time stamp right away (Linux only!)
        timeval now;
        if ( gettimeofday( &now, 0 ) != 0 ) {
            stringstream ss;
            ss << "Pproblem getting timeofday: " << strerror(errno) << endl;
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,ss.str() );
        }
    
        // zero tolerance to serial errors
        if ( ret<0 )
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO, "Driver: Timeout reading from serial port" );
        if ( ret==0 )
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,"Driver: Read 0 bytes from serial port");
    
        // 
        // We successfully read something from the serial port
        //
        tracer_.debug( serialData, 10 );
    
        //Put it into the message object and checksum the data
        try {
            // This throws if it cannot find the * to deliminate the checksum field
            nmeaMessage.setSentence( serialData.c_str(), gbxgpsutilacfr::TestChecksum );
        }
        catch ( const gbxgpsutilacfr::NmeaException& e ) {
            //Don't throw on isolated checksum problems
            if ( nmeaExceptionCount++ < 3 ) {
                continue;
            }
            stringstream ss;
            ss << "Problem reading from GPS: " << e.what();
            tracer_.error( ss.str() );
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,ss.str());
        }
        nmeaExceptionCount = 0;
    
        // Only populate the data structures if our message passes the checksum!
        if ( !nmeaMessage.haveValidChecksum() ) {            
            // Dont throw an exception on the first failed checksum.
            if ( nmeaFailChecksumCount++ < 3 ) {
                tracer_.warning("Gps driver: Single message failed checksum. Not throwing an exception yet!" );
                continue;
            }
            else {
                throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,"More than 3 sequential messages failed the checksum");
            }
        }
        nmeaFailChecksumCount = 0;

        //First split up the data fields in the string we have read.
        nmeaMessage.parseTokens();
        
        //We should not get any messages with failed checksums, but just in case
        if( nmeaMessage.haveTestedChecksum() && (!nmeaMessage.haveValidChecksum()) ) {
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,"Driver: Message fails checksum");
        }
        
        //And then find out which type of messge we have recieved...
        string MsgType = nmeaMessage.getDataToken(0);
        
        if ( MsgType == "$GPGGA" ) {
            tracer_.debug("got GGA message",4);
            if ( !config_.readGga )
                continue;
            genericData.reset( extractGgaData( nmeaMessage, now.tv_sec, now.tv_usec ) );
            break;
        }
        else if ( MsgType == "$GPVTG" ) {
            tracer_.debug("got VTG message",4);
            if ( !config_.readVtg )
                continue;
            genericData.reset( extractVtgData( nmeaMessage, now.tv_sec, now.tv_usec ) );
            break;
        }
        else if ( MsgType == "$PGRME" ) {
            tracer_.debug("got RME message",4);
            if ( !config_.readRme )
                continue;
            genericData.reset( extractRmeData( nmeaMessage, now.tv_sec, now.tv_usec ) );
            break;
        }
        else if ( MsgType == "$PGRMO" ) {
            //This message is sent by us to control msg transmission and then echoed by GPS
            //So we can just ignore it
            continue;
        }
        else {
            // if we get here the msg is unknown
            stringstream  ErrMsg; 
            ErrMsg << "Message type unknown " << MsgType <<endl; 
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,ErrMsg.str());
        } 

    }

    return genericData;
}
