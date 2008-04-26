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
// #include <IceUtil/IceUtil.h>
// #include <gbxsickacfr/gbxiceutilacfr/gbxiceutilacfr.h>
#include <cstdlib>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

#include "driver.h"

using namespace std;
using namespace gbxgarminacfr;

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
    ss << "Garmin driver config: device="<<device;
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
        ss << __func__ << "(): Invalid config: " << config_.toString();
        throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }

    stringstream ssDebug;
    ssDebug << "Connecting to GPS on serial port " << config_.device;
    tracer_.debug( ssDebug.str() );

    // there's no need to make this configurable
//     int baud = context_.properties().getPropertyAsIntWidDefault( prefix+"Baud", 4800 );
    int baud = 4800;

    // according to Duncan, the first 5 initialization messages come in 1 sec.
    // 2 secs should be conservative.
    serial_.reset( new gbxserialacfr::Serial( config_.device, baud, gbxserialacfr::Serial::Timeout(2,0) ) );

    init();
}

Driver::~Driver()
{
    disableDevice();
}

void 
Driver::read( Data &data )
{  
    return readFrame( data );
}

void
Driver::init()
{ 
    //Make sure that we clear our internal data structures
    memset((void*)(&nmeaMessage_) , 0 , sizeof(nmeaMessage_));
    memset((void*)(&gpsData_) , 0 , sizeof(gpsData_));

    try {
        enableDevice();
        //TODO Need to check here that we have been successful.
        clearFrame();
    }
    catch ( const gbxserialacfr::SerialException &e )
    {
        stringstream ss;
        ss << "Driver: Caught SerialException: " << e.what();
        tracer_.error( ss.str() );
        throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }
}

//*****************************************************************************

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



//***********************************************************************
void
Driver::disableDevice()
{

    //Simply send the no messages command!
    gbxgpsutilacfr::NmeaMessage DisableAllMsg("$PGRMO,,2*xx\r\n",gbxgpsutilacfr::AddChecksum);
    serial_->writeString(DisableAllMsg.sentence());
}




//****************************************************************************
// Read one complete frame of data. IE all the messages that we need before returning the data.

void
Driver::readFrame(Data& GpsData)
{
    
    char serial_data[1024];
    int gpsMsgNotYetGotFrameCount = 0;

    //How many messages are we looking for to make our frame
    const int N_MSGS_IN_FRAME = 3;
    
    //Clear our data before we start trying to assemble the frame
    clearFrame();

    
    while(! haveCompleteFrame() ){
        
        // This will block up to the timeout
        tracer_.debug( "Driver::read(): calling serial_->readLine()", 10 );
        int ret = serial_->readLine(serial_data,1024);
        tracer_.debug( serial_data, 10 );

//         timeOfRead_ = IceUtil::Time::now();
//         gbxsickacfr::gbxiceutilacfr::now( timeOfReadSec_, timeOfReadUsec_ );   
        timeval now;
        if ( gettimeofday( &now, 0 ) != 0 ) {
            stringstream ss;
            ss << "Pproblem getting timeofday: " << strerror(errno) << endl;
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,ss.str() );
        }

        if ( ret<0 ) {
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO, "Driver: Timeout reading from serial port" );
        }
    
        if(ret==0) {
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,"Driver: Read 0 bytes from serial port");
        }
    
        // 
        // We successfully read something from the serial port
        //
    

        //Put it into the message object and checksum the data
        static int nmeaExceptionCount =0;
        try{
            //This throws if it cannot find the * to deliminate the checksum field
            nmeaMessage_.setSentence(serial_data,gbxgpsutilacfr::TestChecksum);
        }
        catch (gbxgpsutilacfr::NmeaException &e){
        //Don't throw if only occasional messages are missing the checksums
            if(nmeaExceptionCount++ < 3) {return;}
            stringstream ss;
            ss << "MainThread: Problem reading from GPS: " << e.what();
            tracer_.error( ss.str() );
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,ss.str());
        }
        nmeaExceptionCount = 0;
    
        //Only populate the data structures if our message passes the checksum!
        static int nmeaFailChecksumCount =0;
        if(nmeaMessage_.haveValidChecksum()){        
            nmeaFailChecksumCount = 0;
            addDataToFrame();
        }else{      
            if(nmeaFailChecksumCount++ >= 3){ //Dont throw an exception on the first failed checksum.
                throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,"Driver: more than 3 sequential messages failed the checksum\n");
            }else{
                tracer_.error("Driver: Single message failed checksum. Not throwing an exception yet!\n" );
            }
        }

        //Make sure that we do not wait for ever trying to get a frame of data
        //Note that we might need to skip the N * $PGRMO messages echoed back from receiver when starting
        //As well as the N * messages that we are looking for
        if(gpsMsgNotYetGotFrameCount++ >= (N_MSGS_IN_FRAME * 3)){
            throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,"Driver: Not able to assemble a complete data frame\n");
        }

    }

    tracer_.debug("GPS got a complete frame\n", 10 );

    // Hand the data back to the outside world
    GpsData=gpsData_;
}




//**********************************************************************************

void
Driver::addDataToFrame()
{
    //First split up the data fields in the string we have read.
    nmeaMessage_.parseTokens();
    
    //We should not be being passed any messages with failed checksums, but just in case
    if(nmeaMessage_.haveTestedChecksum() && (!nmeaMessage_.haveValidChecksum())){
        throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,"Driver: Message fails checksum");
    }

    //And then find out which type of messge we have recieved...
    string MsgType = nmeaMessage_.getDataToken(0);
    
    if(MsgType == "$GPGGA"){
        tracer_.debug("got GGA message\n",4);
        extractGGAData();
        haveGGA_ = true;
        return;
    }else if(MsgType == "$GPVTG"){
        tracer_.debug("got VTG message\n",4);
        extractVTGData();
        haveVTG_ = true;
        return;
    }else if(MsgType == "$PGRME"){
        tracer_.debug("got RME message\n",4);
        extractRMEData();
        haveRME_ = true;
        return;
    }else if(MsgType == "$PGRMO"){
        //This message is sent by us to control msg transmission and then echoed by GPS
        //So we can just ignore it
        return;
    }else{
        // if we get here the msg is unknown
        stringstream  ErrMsg; 
        ErrMsg << "Message type unknown " << MsgType <<endl; 
        throw gbxsickacfr::gbxutilacfr::Exception( ERROR_INFO,ErrMsg.str());
    } 
}


//**************************************************************************************
// Get the useful bits from a GGA message

void 
Driver::extractGGAData(void){

    //Names for the tokens in the GGA message
    enum GGATokens{MsgType=0,UTC,Lat,LatDir,Lon,LonDir,FixType,
                   NSatsUsed,HDOP,Hgt,M1,GeoidHgt,M2,DiffAge,DiffId};

    //cout << nmeaMessage_.sentence()<<endl;

    //position fix type
    switch (nmeaMessage_.getDataToken(FixType)[0])
    {
    case '0': 
        gpsData_.positionType = GpsPositionTypeNotAvailable;
        return;
    case '1': 
        gpsData_.positionType = GpsPositionTypeAutonomous;
        break;
    case '2': 
        gpsData_.positionType = GpsPositionTypeDifferential;
        break;
    }
    
//     gpsData_.timeStamp = orcaice::toOrcaTime (timeOfRead_);
//     gbxiceutil::timeFromIceUtil( timeOfRead_, gpsData_.timeStampSec, gpsData_.timeStampUsec );
    gpsData_.timeStampSec = timeOfReadSec_;
    gpsData_.timeStampUsec = timeOfReadUsec_;
    
    //UTC time 
    sscanf(nmeaMessage_.getDataToken(UTC).c_str(),"%02d%02d%lf",
           &gpsData_.utcTimeHrs, &gpsData_.utcTimeMin, &gpsData_.utcTimeSec );
    //position
    int deg;
    double min;
    double dir;
    
    //latitude
    sscanf(nmeaMessage_.getDataToken(Lat).c_str(),"%02d%lf",&deg,&min);
    dir = (*nmeaMessage_.getDataToken(LatDir).c_str()=='N') ? 1.0 : -1.0;
    gpsData_.latitude=dir*(deg+(min/60.0));
    //longitude
    sscanf(nmeaMessage_.getDataToken(Lon).c_str(),"%03d%lf",&deg,&min);
    dir = (*nmeaMessage_.getDataToken(LonDir).c_str()=='E') ? 1.0 : -1.0;
    gpsData_.longitude=dir*(deg+(min/60.0));
    
    //number of satellites in use
    gpsData_.satellites = atoi(nmeaMessage_.getDataToken(NSatsUsed).c_str());
    
    //altitude
    gpsData_.altitude=atof(nmeaMessage_.getDataToken(Hgt).c_str());
    
    //geoidal Separation
    gpsData_.geoidalSeparation=atof(nmeaMessage_.getDataToken(GeoidHgt).c_str());
    

    //cout << "Lat " << GpsData_.latitude << " Long " << GpsData_.longitude ;
    //cout << " Hght "<< GpsData_.altitude << " Geoid "<< GpsData_.geoidalSeparation << endl;

    // Set flag
    
    return;
}


//********************************************************************
// VTG provides velocity and heading information
void 
Driver::extractVTGData(void){

    //Names for the VTG message items
    enum VTGTokens{MsgType=0,HeadingTrue,T,HeadingMag,M,SpeedKnots,
                   N,SpeedKPH,K,ModeInd};

    //Check for an empty string. Means that we are not moving
    //When the message has empty fields tokeniser skips so we get the next field inline.
    if(nmeaMessage_.getDataToken(HeadingTrue)[0] == 'T' ){
        gpsData_.speed=0.0;
        gpsData_.climbRate=0.0;
        gpsData_.heading=0.0;
        return;
    }

    //heading
    double headingRad = DEG2RAD(atof(nmeaMessage_.getDataToken(HeadingTrue).c_str()));
    NORMALISE_ANGLE( headingRad );
    gpsData_.heading=headingRad;
    //speed - converted to m/s
    gpsData_.speed=atof(nmeaMessage_.getDataToken(SpeedKPH).c_str());
    gpsData_.speed*=(1000/3600.0);
    //set to zero
    gpsData_.climbRate=0.0;
    
    //cout << nmeaMessage_.sentence() << endl;
    // cout << "head "<< RAD2DEG(GpsData_.heading) << " speed " << GpsData_.speed << endl;
    
    return;
}


//*********************************************************************************************
// RME message. This one is garmin specific... Give position error estimates
// See the file garminErrorPositionEstimate.txt for a discussion of the position errors as
// reported here. Essentially the EPE reported by the garmin is a 1 sigma error (RMS) or a
// 68% confidence bounds.

void 
Driver::extractRMEData(void){
    //Names for the RME message items
    enum VTGTokens{MsgType=0,HError,M1,VError,M2,EPE,M3};
    
    gpsData_.horizontalPositionError = atof(nmeaMessage_.getDataToken(HError).c_str());
    gpsData_.verticalPositionError = atof(nmeaMessage_.getDataToken(VError).c_str());
    
    //cout << nmeaMessage_.sentence() << endl;
    //cout << "Herr " << GpsData_.horizontalPositionError << " Verr " << GpsData_.verticalPositionError<<endl;

    return;

}
