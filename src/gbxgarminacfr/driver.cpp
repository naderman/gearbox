/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2009 Duncan Mercer, Alex Brooks, Alexei Makarenko, Tobias Kaupp, Michael Warren
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <bitset>
#include <iostream>
#include <sstream>
#include <gbxutilacfr/gbxutilacfr.h>
#include "nmea.h"
#include <cstdlib>
#include <cstring>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

#include "driver.h"

using namespace std;

///////////////////////////////////////

namespace gbxgarminacfr {

namespace {

// Get the useful bits from a GGA message
GenericData* extractGgaData( const gbxgpsutilacfr::NmeaMessage& msg, int timeSec, int timeUsec )
{
    std::auto_ptr<GgaData> data( new GgaData );

    data->timeStampSec = timeSec;
    data->timeStampUsec = timeUsec;

    //Names for the tokens in the GGA message
    enum GgaTokens{MsgType=0,UTC,Lat,LatDir,Lon,LonDir,FixType,
                   NSatsUsed,HDOP,Hgt,M1,GeoidHgt,M2,DiffAge,DiffId};

    //UTC time 
    sscanf(msg.getDataToken(UTC).c_str(),"%02d%02d%lf",
           &data->utcTimeHrs, &data->utcTimeMin, &data->utcTimeSec );

    //number of satellites in use
    data->satellites = atoi(msg.getDataToken(NSatsUsed).c_str());

    // fix type
    switch ( msg.getDataToken(FixType)[0] )
    {
    case '0': 
        data->fixType = Invalid;
        // NOTE: not processing the rest!
        data->latitude = 0.0;
        data->longitude = 0.0;
        data->altitude = 0.0;
        data->geoidalSeparation = 0.0;
        return data.release();
    case '1': 
        data->fixType = Autonomous;
        break;
    case '2': 
        data->fixType = Differential;
        break;
    default :
        throw gbxutilacfr::Exception( ERROR_INFO, "GGA sentence contains unknown GPS fix type: '"+msg.getDataToken(FixType)+"'" );
    }
        
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
    
    //altitude
    data->isAltitudeKnown = !msg.isDataTokenEmpty(Hgt);
    if ( data->isAltitudeKnown )
        data->altitude=atof(msg.getDataToken(Hgt).c_str());
    
    //geoidal Separation
    data->geoidalSeparation=atof(msg.getDataToken(GeoidHgt).c_str());

    return data.release();
}

// VTG provides velocity and heading information
GenericData* extractVtgData( const gbxgpsutilacfr::NmeaMessage& msg, int timeSec, int timeUsec )
{
    std::auto_ptr<VtgData> data( new VtgData );

    data->timeStampSec = timeSec;
    data->timeStampUsec = timeUsec;

    //Names for the VTG message items
    enum VtgTokens{MsgType=0,HeadingTrue,T,HeadingMag,M,SpeedKnots,
                   N,SpeedKPH,K,ModeInd};

    //Check for an empty string. Means that we can't tell anything useful.
    if ( msg.isDataTokenEmpty(HeadingTrue) )
    {
        data->isValid = false;
        data->headingTrue = 0.0;
        data->headingMagnetic = 0.0;
        data->speed = 0.0;
        return data.release();
    }
    data->isValid = true;

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

    return data.release();
}

// RME message. This one is garmin specific... Give position error estimates
// See doc.dox for a discussion of the position errors as reported here. 
// Essentially the EPE reported by the garmin is a 1 sigma error (RMS) or a
// 68% confidence bounds.
GenericData* extractRmeData( const gbxgpsutilacfr::NmeaMessage& msg, int timeSec, int timeUsec )
{
    std::auto_ptr<RmeData> data( new RmeData );

    data->timeStampSec = timeSec;
    data->timeStampUsec = timeUsec;

    //Names for the RME message items
    enum RmeTokens{MsgType=0,HError,M1,VError,M2,EPE,M3};
    
    if ( msg.isDataTokenEmpty(HError) )
    {
        // No valid information
        data->isValid                      = false;
        data->isVerticalPositionErrorValid = false;
        data->horizontalPositionError      = 0.0;
        data->verticalPositionError        = 0.0;
        data->estimatedPositionError       = 0.0;
        return data.release();
    }
    data->isValid = true;

    data->horizontalPositionError = atof(msg.getDataToken(HError).c_str());

    if ( msg.isDataTokenEmpty(VError) )
    {
        data->isVerticalPositionErrorValid = false;
        data->verticalPositionError = -1;
    }
    else
    {
        data->isVerticalPositionErrorValid = true;
        data->verticalPositionError = atof(msg.getDataToken(VError).c_str());
    }

    data->estimatedPositionError = atof(msg.getDataToken(EPE).c_str());
    return data.release();
}

// RMC provides a combination of GGA and VTG data
GenericData* extractRmcData( const gbxgpsutilacfr::NmeaMessage& msg, int timeSec, int timeUsec )
{
    std::auto_ptr<RmcData> data( new RmcData );

    data->timeStampSec = timeSec;
    data->timeStampUsec = timeUsec;

    //Names for the RMC message items
    enum RmcTokens{MsgType=0,UTC,Status,Lat,LatDir,Lon,LonDir,SpeedKnots,
                    HeadingTrue,DiffAge,MagneticVar,MagneticDir,ModeInd};

    //Check for an empty string. Means that we can't tell anything useful.
    if ( msg.isDataTokenEmpty(HeadingTrue) )
    {
        data->isValid = false;
        data->headingTrue = 0.0;
        data->headingMagnetic = 0.0;
        data->speed = 0.0;
        return data.release();
    }
    data->isValid = true;

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

    // true heading
    double headingRad = DEG2RAD(atof(msg.getDataToken(HeadingTrue).c_str()));
    NORMALISE_ANGLE( headingRad );
    data->headingTrue=headingRad;

    // magnetic heading
    double headingVar;
    try {
        // Magnetic deviation from true
        headingVar = DEG2RAD(atof(msg.getDataToken(MagneticVar).c_str()));
        // Direction of magnetic deviation
        if (*msg.getDataToken(MagneticDir).c_str() == 'E')
            headingRad -= headingVar;
        else 
            headingRad += headingVar;
    }
    catch ( const gbxgpsutilacfr::NmeaException& e ) {
        // If this occurs, cannot get magnetic heading. Set it to 0.
        headingRad = 0.0;
    }
    NORMALISE_ANGLE( headingRad );
    data->headingMagnetic=headingRad;

    //speed - converted from knots to m/s
    data->speed=atof(msg.getDataToken(SpeedKnots).c_str());
    // knots to kph
    data->speed*=1.852;
    // kph to m/s
    data->speed*=(1000/3600.0);

    return data.release();
}

}

///////////////////////////////////////

bool
Config::isValid() const
{
    if ( device.empty() ) 
        return false;

    if ( !( readGga || readVtg || readRme ) )
        return false;

    return true;
}

std::string
Config::toString() const
{
    std::stringstream ss;
    ss << "Garmin driver config: " << endl 
       << "\tdevice="<<device << endl
       << "\twill read sentences: GPGGA="<<readGga<<" GPVTG="<<readVtg<<" PGRME="<<readRme<<" GPRMC="<<readRmc;
    return ss.str();
}

///////////////////////////////////////

Driver::Driver( const Config &config, 
            gbxutilacfr::Tracer &tracer,
            gbxutilacfr::Status &status ) :
    config_(config),
    tracer_(tracer),
    status_(status)
{
    if ( !config_.isValid() )
    {
        stringstream ss;
        ss << "Invalid config: " << config_.toString();
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
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
    // Don't throw from destructors...
    try {
        disableDevice();
    }
    catch ( const std::exception &e )
    {
        cout << "Driver::~Driver: exception while disabling: " << e.what() << endl;
    }
    catch ( ... ) {}
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
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }
}

void
Driver::enableDevice()
{
    stringstream ss;
    ss << "Configure GPS device. Protocol: " << config_.protocol;
    tracer_.info(ss.str());

    // Non-Garmin devices do not support message enabling and disabling
    if (config_.protocol != "Garmin") return;

    //Create the messages that we are going to send and add the checksums
    //Note that the checksum field is filled with 'x's before we start

    //First disables all output messages then enable selected ones only.
    tracer_.debug( "Driver::enableDevice(): calling disableAllMsg", 10 );
    gbxgpsutilacfr::NmeaMessage disableAllMsg( "$PGRMO,,2*xx\r\n",gbxgpsutilacfr::AddChecksum );
    serial_->writeString( disableAllMsg.sentence() );

    // alexb: what is this sleep for?
    sleep(1);
    
    if ( config_.readGga ) {
        tracer_.debug( "Driver::enableDevice(): calling enableGgaMsg", 10 );
        gbxgpsutilacfr::NmeaMessage enableGgaMsg( "$PGRMO,GPGGA,1*xx\r\n",gbxgpsutilacfr::AddChecksum );
        serial_->writeString( enableGgaMsg.sentence() );
    }

    if ( config_.readVtg ) {
        tracer_.debug( "Driver::enableDevice(): calling enableVtgMsg", 10 );
        gbxgpsutilacfr::NmeaMessage enableVtgMsg( "$PGRMO,GPVTG,1*xx\r\n",gbxgpsutilacfr::AddChecksum );
        serial_->writeString( enableVtgMsg.sentence() );
    }

    if ( config_.readRme ) {
        tracer_.debug( "Driver::enableDevice(): calling enableRmeMsg", 10 );
        gbxgpsutilacfr::NmeaMessage enableRmeMsg( "$PGRMO,PGRME,1*xx\r\n",gbxgpsutilacfr::AddChecksum );
        serial_->writeString( enableRmeMsg.sentence() );
    }
    if ( config_.readRmc ) {
        tracer_.debug( "Driver::enableDevice(): calling enableRmcMsg", 10 );
        gbxgpsutilacfr::NmeaMessage enableRmcMsg( "$PGRMO,GPRMC,1*xx\r\n",gbxgpsutilacfr::AddChecksum );
        serial_->writeString( enableRmcMsg.sentence() );
    }

    // alexb: what is this sleep for?
    sleep(1);
}

void
Driver::disableDevice()
{
    // Simply send the no messages command!
    gbxgpsutilacfr::NmeaMessage disableAllMsg( "$PGRMO,,2*xx\r\n",gbxgpsutilacfr::AddChecksum );
    serial_->writeString( disableAllMsg.sentence() );
}

std::auto_ptr<GenericData>  
Driver::read()
{  
    std::auto_ptr<GenericData> genericData;
    gbxgpsutilacfr::NmeaMessage nmeaMessage;

    int nmeaExceptionCount = 0;
    int nmeaFailChecksumCount = 0;

    while ( true )
    {
        string serialData;
    
        //
        // This will block up to the timeout
        //
        tracer_.debug( "Driver::read(): calling serial_->readLine()", 10 );
        int ret = serial_->readLine( serialData );
    
        // get time stamp right away (Linux only!)
        timeval now;
        if ( gettimeofday( &now, 0 ) != 0 ) {
            stringstream ss;
            ss << "Problem getting timeofday: " << strerror(errno) << endl;
            throw gbxutilacfr::Exception( ERROR_INFO,ss.str() );
        }
    
        // zero tolerance to serial errors
        if ( ret<0 )
            throw gbxutilacfr::Exception( ERROR_INFO, "Driver: Timeout reading from serial port" );
        if ( ret==0 )
            throw gbxutilacfr::Exception( ERROR_INFO,"Driver: Read 0 bytes from serial port");
    
        // 
        // We successfully read something from the serial port
        //
        tracer_.debug( serialData, 10 );

        // remove the trailing new line character
        assert( *(serialData.end()-1) == '\n' && "Driver: Serial data did not end in '\n'" );
        serialData.erase( serialData.end()-1 );
    
        // dealing with unexplained cntrl characters in the message
/*
        for ( string::iterator it=serialData.begin(); it!=serialData.end(); ++it ) {
            if ( iscntrl( *it ) ) {
                // debug
                cout<<now.tv_sec<<" "<<now.tv_usec<<" "<<" before='"<<serialData<<"'"<<endl;
                cout<<now.tv_sec<<" "<<now.tv_usec<<" removing control char="<<std::hex<<(unsigned int)*it<<std::dec<<endl;
                it = serialData.erase( it );
                cout<<now.tv_sec<<" "<<now.tv_usec<<" "<<" after='"<<serialData<<"'"<<endl;
            }
        }
*/
        //Put it into the message object and checksum the data
        try {
            // This throws if it cannot find the * to deliminate the checksum field
            nmeaMessage.setSentence( serialData, gbxgpsutilacfr::TestChecksum );
        }
        catch ( const gbxgpsutilacfr::NmeaException& e ) {
            //Don't throw on isolated checksum problems
            if ( nmeaExceptionCount++ < 3 ) {
                continue;
            }
            stringstream ss;
            ss << "Problem reading from GPS: " << e.what();
            tracer_.error( ss.str() );
            throw gbxutilacfr::Exception( ERROR_INFO,ss.str());
        }
        nmeaExceptionCount = 0;

        // debug
//         if ( nmeaFailChecksumCount>0 )
//             cout<<now.tv_sec<<" "<<now.tv_usec<<" "<<nmeaFailChecksumCount<<" the sentence AFTER, raw='"<<serialData<<"'"<<endl<<endl;
    
        // Only populate the data structures if our message passes the checksum!
        const int nmeaFailChecksumMaxCount = 10;
        if ( !nmeaMessage.haveValidChecksum() ) {            
            // Dont throw an exception on the first failed checksum.
            if ( nmeaFailChecksumCount++ < nmeaFailChecksumMaxCount ) {
//                 tracer_.warning("Gps driver: Single message failed checksum. Not throwing an exception yet!" );
                // debug
                cout<<now.tv_sec<<" "<<now.tv_usec<<" "<<nmeaFailChecksumCount<<endl;
//                     <<" sentence='"<<nmeaMessage.sentence()<<"' raw='"<<serialData<<"'"<<endl;
                continue;
            }
            else {
                stringstream ss; 
                ss << "More than "<<nmeaFailChecksumMaxCount<<" sequential messages failed the checksum";
                throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
            }
        }
        nmeaFailChecksumCount = 0;

        //First split up the data fields in the string we have read.
        nmeaMessage.parseTokens();
        
        // We should not get any messages with failed checksums, but just in case
        if( nmeaMessage.haveTestedChecksum() && (!nmeaMessage.haveValidChecksum()) ) {
            throw gbxutilacfr::Exception( ERROR_INFO,"Driver: Message fails checksum");
        }
        
        // And then find out which type of messge we have recieved...
        string MsgType = nmeaMessage.getDataToken(0);
        
        if ( MsgType == "$GPGGA" ) {
            if ( config_.readGga )
                tracer_.debug("got GPGGA message",4);
            else
                throw gbxutilacfr::Exception( ERROR_INFO, "got unexpected GPGGA message" );
            genericData.reset( extractGgaData( nmeaMessage, now.tv_sec, now.tv_usec ) );
            if ( genericData.get() )
                break;
            else
                continue;
        }
        else if ( MsgType == "$GPVTG" ) {
            if ( config_.readVtg )
                tracer_.debug("got GPVTG message",4);
            else
                throw gbxutilacfr::Exception( ERROR_INFO, "got unexpected GPVTG message" );
            genericData.reset( extractVtgData( nmeaMessage, now.tv_sec, now.tv_usec ) );
            if ( genericData.get() )
                break;
            else
                continue;
        }
        else if ( MsgType == "$PGRME" ) {
            if ( config_.readRme )
                tracer_.debug("got PGRME message",4);
            else
                throw gbxutilacfr::Exception( ERROR_INFO, "got unexpected PGRME message" );
            genericData.reset( extractRmeData( nmeaMessage, now.tv_sec, now.tv_usec ) );
            if ( genericData.get() )
                break;
            else
                continue;
        }
        else if ( MsgType == "$GPRMC" ) {
            if ( config_.readRmc )
                tracer_.debug("got GPRMC message",4);
            else
                throw gbxutilacfr::Exception( ERROR_INFO, "got unexpected GPRMC message" );
            genericData.reset( extractRmcData( nmeaMessage, now.tv_sec, now.tv_usec ) );
            if ( genericData.get() )
                break;
            else
                continue;
        }
        else if ( MsgType == "$PGRMO" ) {
            //This message is sent by us to control msg transmission and then echoed by GPS
            //So we can just ignore it and wait for the next one.
            continue;
        }
        else {
            stringstream ss; ss << "Message type unknown " << MsgType <<endl; 
            // if we get here the msg is unknown
            if ( config_.ignoreUnknown )
                tracer_.debug( ss.str() );
            else
                throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
        } 
    }
    return genericData;
}

std::string toString( const FixType &f )
{
    switch ( f )
    {
    case Invalid: return "Invalid";
    case Autonomous: return "Autonomous";
    case Differential: return "Differential";
    default: return "??";        
    }
}

std::string toString( const GgaData &d )
{
    stringstream ss;
    ss << endl;
    ss << "  timeStampSec                 : " << d.timeStampSec << endl
       << "  timeStampUsec                : " << d.timeStampUsec << endl
       << "  utcTimeHrs                   : " << d.utcTimeHrs << endl
       << "  utcTimeMin                   : " << d.utcTimeMin << endl
       << "  utcTimeSec                   : " << d.utcTimeSec << endl
       << "  latitude                     : " << d.latitude << endl
       << "  longitude                    : " << d.longitude << endl
       << "  isAltitudeKnown              : " << d.isAltitudeKnown << endl
       << "  altitude                     : " << d.altitude << endl
       << "  fixType                      : " << d.fixType << endl
       << "  satellites                   : " << d.satellites << endl
       << "  horizontalDilutionOfPosition : " << d.horizontalDilutionOfPosition << endl
       << "  geoidalSeparation            : " << d.geoidalSeparation;
    return ss.str();
}
std::string toString( const VtgData &d )
{
    stringstream ss;
    ss << endl;
    ss << "  timeStampSec    : " << d.timeStampSec << endl
       << "  timeStampUsec   : " << d.timeStampUsec << endl
       << "  isValid         : " << d.isValid << endl
       << "  headingTrue     : " << d.headingTrue << endl 
       << "  headingMagnetic : " << d.headingMagnetic << endl 
       << "  speed           : " << d.speed;
    return ss.str();
}
std::string toString( const RmeData &d )
{
    stringstream ss;
    ss << endl;
    ss << "  timeStampSec                 : " << d.timeStampSec << endl
       << "  timeStampUsec                : " << d.timeStampUsec << endl
       << "  isValid                      : " << d.isValid << endl
       << "  horizontalPositionError      : " << d.horizontalPositionError << endl
       << "  isVerticalPositionErrorValid : " << d.isVerticalPositionErrorValid << endl
       << "  verticalPositionError        : " << d.verticalPositionError << endl
       << "  estimatedPositionError       : " << d.estimatedPositionError;
    return ss.str();
}

std::string toString( const RmcData &d )
{
    stringstream ss;
    ss << endl;
    ss << "  timeStampSec    : " << d.timeStampSec << endl
       << "  timeStampUsec   : " << d.timeStampUsec << endl
       << "  utcTimeHrs      : " << d.utcTimeHrs << endl
       << "  utcTimeMin      : " << d.utcTimeMin << endl
       << "  utcTimeSec      : " << d.utcTimeSec << endl
       << "  latitude        : " << d.latitude << endl
       << "  longitude       : " << d.longitude << endl
       << "  isValid         : " << d.isValid << endl
       << "  headingTrue     : " << d.headingTrue << endl 
       << "  headingMagnetic : " << d.headingMagnetic << endl 
       << "  speed           : " << d.speed;
    return ss.str();
}

}
