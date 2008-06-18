/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Matthew Ridley, Ben Upcroft, Michael Moser
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <gbxnovatelacfr/driver.h>
#include <gbxnovatelacfr/gbxnovatelutilacfr/serialconnectivity.h>
#include <gbxnovatelacfr/gbxnovatelutilacfr/novatelmessages.h>
#include <gbxnovatelacfr/gbxnovatelutilacfr/imudecoder.h>
#include <gbxnovatelacfr/gbxnovatelutilacfr/receiverstatusdecoder.h>
#include <gbxnovatelacfr/gbxnovatelutilacfr/crc32.h>

#include <gbxserialacfr/gbxserialacfr.h>
#include <gbxutilacfr/gbxutilacfr.h>

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <vector>

#include <sys/time.h>
#include <time.h>

using namespace std;
using namespace gbxserialacfr;

namespace gnua = gbxnovatelutilacfr;
namespace gna = gbxnovatelacfr;

namespace {
// binary messages defined by novatel
#pragma pack(push,1)
    union novatelMessage{
        struct{
            gnua::Oem4BinaryHeader header;
            char               data[484];
        };
        struct{
            gnua::Oem4ShortBinaryHeader shortHeader;
            char               shortData[500];
        };
        unsigned char rawMessage[512];

        // these guys are used to directly decode messages;
        // obviously fails on endian mismatch, any sort of size mismatch and is rather nasty in general;
        // feel free to implement something better
        gnua::BestGpsPosLogB bestGpsPos;
        gnua::BestGpsVelLogB bestGpsVel;
        gnua::InsPvaLogSB   insPva;
        gnua::RawImuLogSB   rawImu;
    };
#pragma pack(pop)
    int readNovatelMessage(union novatelMessage &msg, struct timeval &timeStamp, gbxserialacfr::Serial *serial);
    std::auto_ptr<gna::GenericData> createExternalMsg(gnua::InsPvaLogSB &insPva, struct timeval &timeStamp);
    std::auto_ptr<gna::GenericData> createExternalMsg(gnua::BestGpsPosLogB &bestGpsPos, struct timeval &timeStamp);
    std::auto_ptr<gna::GenericData> createExternalMsg(gnua::BestGpsVelLogB &bestGpsVel, struct timeval &timeStamp);
    std::auto_ptr<gna::GenericData> createExternalMsg(gnua::RawImuLogSB &rawImu, struct timeval &timeStamp, gnua::ImuDecoder *imuDecoder);

    std::string doubleVectorToString(vector<double > &vec, std::string seperator = std::string(" "));
}

namespace gbxnovatelacfr
{
Driver::Driver( const Config& cfg,
        gbxutilacfr::Tracer &tracer) :
    serial_(0),
    baud_(115200),
    config_(cfg),
    tracer_(tracer)
{
    if(false == config_.isValid()){
        throw (std::string("Invalid Configuration!"));
    }

    // configure serial port
    baud_ = config_.baudRate_;
    std::string serialDevice = config_.serialDevice_;
    serial_.reset(new Serial( serialDevice, baud_, Serial::Timeout(1,0) ));
    serial_->setDebugLevel(0);
    if(0 != connectToHardware() ){
        throw (std::string("failed to connect to receiver!"));
    }

    // just in case something is running... stops the novatel logging any messages
    serial_->writeString( "unlogall\r\n" );
    serial_->drain();
    configureImu();
    configureIns();
    configureGps();
    requestData();
    serial_->flush();
    tracer_.info("Setup done, starting normal operation!");
}

Driver::~Driver() {
    // just in case something is running... stops the novatel logging any messages
    try{
        tracer_.info("Stopping NovatelSpan driver!");
        serial_->flush();
        serial_->writeString( "unlogall\r\n" );
        serial_->drain();
        tracer_.info("NovatelSpan driver stopped!");
    }
    catch(...){
        //no throwing from destructors
    }
}

int
Driver::connectToHardware() {
    // baudrates we test for; this is
    // _not_ all the baudrates the receiver
    // can possible be set to
    int baudrates[]={
        9600,
        19200,
        38400,
        57600,
        115200,
        230400
    };
    int currentBaudrate = 0;
    bool correctBaudrate = false;

    std::cout << "Trying to hook up to receiver at different Baudrates\n";
    int maxTry = 4;
    int successThresh = 4;
    int timeOutMsec = 150;
    std::string challenge("unlogall\r\n");
    std::string ack("<OK");
    size_t i=0;
    while(false == correctBaudrate && i<sizeof baudrates/sizeof baudrates[0]){
        currentBaudrate = baudrates[i];
        correctBaudrate = gnua::testConnectivity( challenge, ack, *(serial_.get()), timeOutMsec, maxTry, successThresh, currentBaudrate);
        i++;
    }
    if(false == correctBaudrate){
        std::cout << "\n!Failed to establish a connection to the receiver!\n";
        std::cout << "Check physical connections; Check manually (minicom) for Baudrates < 9600kb/s.\n\n";
        return -1;
    }
    char str[256];
    sprintf( str,"com com1 %d n 8 1 n off on\r\n", baud_ );
    serial_->writeString( str );
    std::cout << "*******************************\n"
        << "** Current Speed " << currentBaudrate << "\n"
        << "** Resetting to " << baud_ << "\n"
        << "*******************************\n";
    std::cout << "** Testing new setting\n** ";
    if(true == gnua::testConnectivity( challenge, ack, *(serial_.get()), timeOutMsec, maxTry, successThresh, baud_)){
        std::cout << "*******************************\n";
        return 0;
    }else{
        std::cout << "*******************************\n";
        return -1;
    }
}

void
Driver::configureImu() {
    int put;

    if(config_.enableImu_){
        imuDecoder_.reset(gnua::createImuDecoder(config_.imuType_));
        std::stringstream ss;
        // tell the novatel what serial port the imu is attached to (com3 == aux)
        put = serial_->writeString( "interfacemode com3 imu imu on\r\n" );
        // the type of imu being used
        ss << "setimutype "
            << config_.imuType_
            << "\r\n";
        put = serial_->writeString( ss.str().c_str() );
        //force the IMU to re-align at every startup
        //put = serial_->writeString( "inscommand reset\r\n" );
        //tracer_.info("Reset IMU; Waiting 5 seconds before continuing!");
        //sleep(5);
    }else{
        // no IMU --> disable INS
        put = serial_->writeString( "inscommand disable\r\n" );
    }
    return;
}

void
Driver::configureIns() {
    int put;
    if(config_.enableSetImuOrientation_ && config_.enableImu_){
        std::stringstream ss;
        // imu orientation constant
        // this tells the imu where its z axis (up) is pointing. constants defined in manual.
        // with imu mounted upside down, constant is 6 and axes are remapped: x = y, y = x, -z = z 
        ss << "setimuorientation " << config_.setImuOrientation_ << "\r\n";
        put = serial_->writeString( ss.str().c_str() );
    }

    if(config_.enableVehicleBodyRotation_ && config_.enableImu_){
        std::stringstream ss;
        // vehicle to imu body rotation
        // angular offset from the vehicle to the imu body. unclear how this relates to imu orientation command 
        // the novatel docs are not especially clear on this stuff; It's highly recommended to mount the IMU
        // exactly as advised by novatel and just ignore this
        ss << "vehiclebodyrotation "
            << config_.vehicleBodyRotation_[0]
            << config_.vehicleBodyRotation_[1]
            << config_.vehicleBodyRotation_[2];
        if(3 == config_.vehicleBodyRotationUncertainty_.size()){
            // optional, vehicle to imu body rotation uncertainty
            ss << config_.vehicleBodyRotationUncertainty_[0]
                << config_.vehicleBodyRotationUncertainty_[1]
                << config_.vehicleBodyRotationUncertainty_[2];
        }
        ss << "\r\n";
        put = serial_->writeString( ss.str().c_str() );
    }

    if(config_.enableImu_){
        std::stringstream ss;
        // The span system kalman fiter needs this info; make _sure_ you do this right
        ss << "setimutoantoffset "
            << config_.imuToGpsOffset_[0]
            << config_.imuToGpsOffset_[1]
            << config_.imuToGpsOffset_[2];

        if( 3 == config_.imuToGpsOffsetUncertainty_.size() ){
            ss << config_.imuToGpsOffsetUncertainty_[0]
                << config_.imuToGpsOffsetUncertainty_[1]
                << config_.imuToGpsOffsetUncertainty_[2];
        }
        ss << "\r\n";
        put = serial_->writeString( ss.str().c_str() );
    }
    return;
}

void
Driver::configureGps() {
    // hardcoded settings first

    // turn off posave as this command implements position averaging for base stations.
    int put = serial_->writeString( "posave off\r\n" );
    // make sure that fixposition has not been set
    put = serial_->writeString( "fix none\r\n" );
    // select the geodetic datum for operation of the receiver (wgs84 = default)
    put = serial_->writeString( "datum wgs84\r\n" );
    //Let the receiver figure out which range corrections are best
    put = serial_->writeString( "PSRDIFFSOURCE AUTO\r\n" );

    // CDGPS
    if(config_.enableCDGPS_){
        tracer_.info("Turning on CDGPS!");
        put = serial_->writeString( "ASSIGNLBAND CDGPS 1547547 4800\r\n" );
    }

    // turn SBAS on/off (essentially global DGPS)
    if(config_.enableSBAS_){
        tracer_.info("Turning on SBAS!");
        put = serial_->writeString( "SBASCONTROL ENABLE Auto 0 ZEROTOTWO\r\n");
        //we try to use WAAS satellites even below the horizon
        put = serial_->writeString( "WAASECUTOFF -5.0\r\n");
    }
    else{
        tracer_.info("Turning off SBAS!");
        put = serial_->writeString( "SBASCONTROL DISABLE Auto 0 NONE\r\n");
    }

    // rtk
    if(config_.enableRTK_){
        tracer_.info("Turning on RTK!");
        put = serial_->writeString( "com com2,9600,n,8,1,n,off,on\r\n" );
        put = serial_->writeString( "interfacemode com2 rtca none\r\n" );
    }

    if(config_.enableUseOfOmniStarCarrier_){
        //Let the receiver figure out which rtk corrections are best
        put = serial_->writeString( "RTKSOURCE AUTO\r\n" );
    }else{
        //We only use our own rtk corrections; _not_ OmniSTAR HP/XP
        put = serial_->writeString( "RTKSOURCE RTCA ANY\r\n" );
    }
    return;
}

void
Driver::requestData() {
    //we assume that the config_ has been checked at this point (isValid())
    //so we don't need to check that the rates make sense
    int put;

    // GPS messages

    // gps position without ins
    if(config_.enableGpsPos_){
        std::stringstream ss;
        ss << "log bestgpsposb ontime " << config_.dtGpsPos_ << "\r\n";
        put = serial_->writeString(ss.str().c_str());
        ss.str("");
        ss << "Turning on GPS position at " << 1.0/config_.dtGpsPos_ << "Hz!";
        tracer_.info(ss.str().c_str());
    }

    // gps velocity without ins
    if(config_.enableGpsVel_){
        std::stringstream ss;
        ss << "log bestgpsvelb ontime " << config_.dtGpsVel_ << "\r\n";
        put = serial_->writeString(ss.str().c_str());
        ss.str("");
        ss << "Turning on GPS velocity at " << 1.0/config_.dtGpsVel_ << "Hz!";
        tracer_.info(ss.str().c_str());
    }


    // INS messages

    // pva data in wgs84 coordinates
    if(config_.enableInsPva_){
        std::stringstream ss;
        ss << "log inspvasb ontime " << config_.dtInsPva_ << "\r\n";
        put = serial_->writeString(ss.str().c_str());
        ss.str("");
        ss << "Turning on INS position/velocity/orientation at " << 1.0/config_.dtInsPva_ << "Hz!";
        tracer_.info(ss.str().c_str());
    }


    // IMU messages

    // raw accelerometer and gyro data
    if(config_.enableRawImu_){
        put = serial_->writeString( "log rawimusb onnew\r\n" );
        tracer_.info("Turning on raw imu data!");
    }

    return;
}

std::auto_ptr<GenericData>
Driver::read(){
    union novatelMessage msg;
    std::auto_ptr<GenericData> data;
    data.reset(0);
    struct timeval timeStamp = {0,0};

    // read msg from hardware
    do{
        // Timeouts are not adjusted once a serial call returns;
        // So we could be stuck here for longer than the set timeout.
        int ret = serial_->bytesAvailableWait();
        if ( ret >= 0 ) {
            switch(readNovatelMessage(msg, timeStamp, serial_.get())){
                case gnua::InsPvaSBLogType:
                    data = createExternalMsg(msg.insPva, timeStamp);
                    break;
                case gnua::BestGpsVelBLogType:
                    data = createExternalMsg(msg.bestGpsVel, timeStamp);
                    break;
                case gnua::BestGpsPosBLogType:
                    data = createExternalMsg(msg.bestGpsPos, timeStamp);
                    break;
                case gnua::RawImuSBLogType:
                    data = createExternalMsg(msg.rawImu, timeStamp, imuDecoder_.get());
                    break;
                default:
                    {
                        std::stringstream ss;
                        ss << "Warning("<<__FILE__<<":"<< __LINE__
                            <<" : got unexpected message from receiver; id: " << msg.header.msgId << std::endl;
                        if(config_.ignoreUnknownMessages_){
                            tracer_.warning(ss.str());
                        }else{
                            throw( ss.str() );
                        }
                    }
                    break;
            }
        }
        else {
            std::stringstream ss;
            ss << "Warning("<<__FILE__<<":"<< __LINE__
             << "Timed out while waiting for data";
            throw (ss.str());
        }
    }while(NULL == data.get()); // repeat till we get valid data

    return data;
}

Config::Config(const SimpleConfig &simpleCfg) :
    serialDevice_(simpleCfg.serialDevice_),
    baudRate_(simpleCfg.baudRate_),
    enableImu_(true),
    imuType_(simpleCfg.imuType_),
    enableInsPva_(true),
    enableGpsPos_(true),
    enableGpsVel_(true),
    enableRawImu_(true),
    ignoreUnknownMessages_(false),
    dtInsPva_(0.02),
    dtGpsPos_(0.2),
    dtGpsVel_(0.2),
    fixInvalidRateSettings_(false),
    imuToGpsOffset_(simpleCfg.imuToGpsOffset_),
    enableInsOffset_(false),
    enableInsPhaseUpdate_(true),
    enableCDGPS_(true),
    enableSBAS_(true),
    enableRTK_(true),
    enableUseOfOmniStarCarrier_(false),
    enableSetImuOrientation_(false),
    enableVehicleBodyRotation_(false) {
}
Config::Config(const GpsOnlyConfig &gpsOnlyCfg) :
    serialDevice_(gpsOnlyCfg.serialDevice_),
    baudRate_(gpsOnlyCfg.baudRate_),
    enableImu_(false),
    enableInsPva_(false),
    enableGpsPos_(true),
    enableGpsVel_(true),
    enableRawImu_(false),
    ignoreUnknownMessages_(false),
    dtGpsPos_(0.05),
    dtGpsVel_(0.05),
    fixInvalidRateSettings_(false),
    enableCDGPS_(true),
    enableSBAS_(true),
    enableRTK_(true),
    enableUseOfOmniStarCarrier_(false) {
}
Config::Config() :
    serialDevice_(""),
    baudRate_(0),
    enableImu_(false),
    imuType_(""),
    enableInsPva_(false),
    enableGpsPos_(false),
    enableGpsVel_(false),
    enableRawImu_(false),
    ignoreUnknownMessages_(false),
    dtInsPva_(1.0),
    dtGpsPos_(1.0),
    dtGpsVel_(1.0),
    fixInvalidRateSettings_(false),
    imuToGpsOffset_(1,0.0),
    imuToGpsOffsetUncertainty_(1,0.0),
    enableInsOffset_(false),
    insOffset_(1,0.0),
    enableInsPhaseUpdate_(false),
    enableCDGPS_(false),
    enableSBAS_(false),
    enableRTK_(false),
    enableUseOfOmniStarCarrier_(false),
    enableSetImuOrientation_(false),
    setImuOrientation_(0),
    enableVehicleBodyRotation_(false),
    vehicleBodyRotation_(1,0.0),
    vehicleBodyRotationUncertainty_(1,0.0) {
}

bool
Config::isValid() const {
    cout << __func__ << "implement me\n";
    return true;
}

std::string
Config::toString(){
    std::stringstream ss;
    ss << "serialDevice_: " << serialDevice_ << " ";
    ss << "baudRate_: " << baudRate_ << " ";
    ss << "enableImu_: " << enableImu_ << " ";
    ss << "imuType_: " << imuType_ << " ";
    ss << "enableInsPva_: " << enableInsPva_ << " ";
    ss << "enableGpsPos_: " << enableGpsPos_ << " ";
    ss << "enableGpsVel_: " << enableGpsVel_ << " ";
    ss << "enableRawImu_: " << enableRawImu_ << " ";
    ss << "ignoreUnknownMessages_: " << ignoreUnknownMessages_ << " ";
    ss << "dtInsPva_: " << dtInsPva_ << " ";
    ss << "dtGpsPos_: " << dtGpsPos_ << " ";
    ss << "dtGpsVel_: " << dtGpsVel_ << " ";
    ss << "fixInvalidRateSettings_: " << fixInvalidRateSettings_ << " ";
    ss << "imuToGpsOffset_: " << doubleVectorToString(imuToGpsOffset_) << " ";
    ss << "imuToGpsOffsetUncertainty_: " << doubleVectorToString(imuToGpsOffsetUncertainty_) << " ";
    ss << "enableInsOffset_: " << enableInsOffset_ << " ";
    ss << "insOffset_: " << doubleVectorToString(insOffset_) << " ";
    ss << "enableInsPhaseUpdate_: " << enableInsPhaseUpdate_ << " ";
    ss << "enableCDGPS_: " << enableCDGPS_ << " ";
    ss << "enableSBAS_: " << enableSBAS_ << " ";
    ss << "enableRTK_: " << enableRTK_ << " ";
    ss << "enableUseOfOmniStarCarrier_: " << enableUseOfOmniStarCarrier_ << " ";
    ss << "enableSetImuOrientation_: " << enableSetImuOrientation_ << " ";
    ss << "setImuOrientation_: " << setImuOrientation_ << " ";
    ss << "enableVehicleBodyRotation_: " << enableVehicleBodyRotation_ << " ";
    ss << "vehicleBodyRotation_: " << doubleVectorToString(vehicleBodyRotation_) << " ";
    ss << "vehicleBodyRotationUncertainty_: " << doubleVectorToString(vehicleBodyRotationUncertainty_);
    return ss.str();
}

bool
SimpleConfig::isValid() const {
    cout << __func__ << "implement me\n";
    return true;
}

std::string
SimpleConfig::toString(){
    std::stringstream ss;
    ss << "serialDevice_: " << serialDevice_ << " ";
    ss << "baudRate_: " << baudRate_ << " ";
    ss << "imuType_: " << imuType_ << " ";
    ss << "imuToGpsOffset_: " << doubleVectorToString(imuToGpsOffset_);
    return ss.str();
}

bool
GpsOnlyConfig::isValid() const {
    cout << __func__ << "implement me\n";
    return true;
}

std::string
GpsOnlyConfig::toString(){
    std::stringstream ss;
    ss << "serialDevice_: " << serialDevice_ << " ";
    ss << "baudRate_: " << baudRate_;
    return ss.str();
}

} //namespace

namespace{
    int
    readNovatelMessage(union novatelMessage &msg, struct timeval &timeStamp, gbxserialacfr::Serial *serial) {
        // read the three sync bytes which are always at the start of the message header
        unsigned short id;
        unsigned long crc;
        unsigned long in_crc;
        msg.header.sb1 = 0;
        int skip = -1;
        int got;

        // read the first sync byte
        do{
            got = serial->readFull( &msg.header.sb1, 1 );
            if ( got <= 0 ) {
                return got;
            }
            if( got>0 ) {
                skip++;
            }
        }while( msg.header.sb1 != 0xaa );

        // get timestamp after the first byte for accuracy
        gettimeofday(&timeStamp, NULL);

        // read the second sync byte
        do {
            got = serial->readFull( &msg.header.sb2, 1 );
            if ( got <= 0 ) {
                return got;
            }
        }while( got!=1 );

        if( msg.header.sb2 != 0x44 ) {
            return -1;
        }

         // read the third sync byte
        do {
            got = serial->readFull( &msg.header.sb3, 1 );
            if ( got <= 0 ) {
                return got;
            }
        }while( got != 1 );

        switch( msg.header.sb3 ) {
            case 0x12: //long packet
                if( // how long is the header ?
                    -1 == serial->readFull( &msg.header.headerLength, 1 )
                    // read all of the header...
                    || -1 == serial->readFull( &msg.header.msgId, msg.header.headerLength-4 )
                    // read the  message data
                    || -1 == serial->readFull( &msg.data, msg.header.msgLength )
                    || -1 == serial->readFull( &in_crc, 4 )
                  ){
                    return -1;
                }

                id = msg.header.msgId;

                crc = gnua::crc( msg.rawMessage,
                                   msg.header.msgLength+msg.header.headerLength );
                break;

            case 0x13: //short packet
                if( // read rest of the header 12 bytes - 3 bytes already read, then the actual data, then the CRC
                    -1 == serial->readFull( &msg.shortHeader.msgLength, 9 )
                    || -1 == serial->readFull( &msg.shortData, msg.shortHeader.msgLength )
                    || -1 == serial->readFull( &in_crc, 4 )
                  ){
                    return -1;
                }

                id = msg.shortHeader.msgId;

                crc = gnua::crc( msg.rawMessage,msg.shortHeader.msgLength + 12 );
                break;

            default: //bollocks
                return -1;
        }

        if(in_crc != crc) {
            fprintf( stderr,"CRC Error: 0x%lx, 0x%lx\n",in_crc,crc );
            throw std::string( "CRC Error" );
            return -1;
        }

        return  id;
    }


    std::auto_ptr<gna::GenericData>
    createExternalMsg(gnua::InsPvaLogSB &insPva, struct timeval &timeStamp){
        //static int cnt;
        //if(0 == cnt++ % 1000) cout << __func__ << " ins; implement me properly!\n";
        gna::InsPvaData *data = new gna::InsPvaData;
        std::auto_ptr<gna::GenericData> genericData( data );

        //data
        data->gpsWeekNr = insPva.data.gpsWeekNr;
        data->secIntoWeek = insPva.data.secIntoWeek;
        data->latitude = insPva.data.latitude;
        data->longitude = insPva.data.longitude;
        data->height = insPva.data.height;
        data->northVelocity = insPva.data.northVelocity;
        data->eastVelocity = insPva.data.eastVelocity;
        data->upVelocity = insPva.data.upVelocity;
        data->roll = insPva.data.roll;
        data->pitch = insPva.data.pitch;
        data->azimuth = insPva.data.azimuth;

        //timestamp
        data->timeStampSec = timeStamp.tv_sec;
        data->timeStampUSec = timeStamp.tv_usec;

        //status
        switch( insPva.data.insStatus ) {
            case 0:
                data->statusMessage = "Ins is inactive";
                data->statusMessageType = gna::Fault;
                break;
            case 1:
                data->statusMessage = "Ins is aligning";
                data->statusMessageType = gna::Warning;
                break;
            case 2:
                data->statusMessage = "Ins solution is bad";
                data->statusMessageType = gna::Warning;
                break;
            case 3:
                data->statusMessage = "Ins solution is good";
                data->statusMessageType = gna::Ok;
                break;
            case 4://fallthrough
            case 5:
                {
                    stringstream ss;
                    ss << "Reserved value?? Check NovatelSpan manual for \"" << insPva.data.insStatus << "\" as INS status";
                    data->statusMessage = ss.str();
                    data->statusMessageType = gna::Warning;
                }
                break;
            case 6:
                data->statusMessage = "Bad Ins Gps agreement";
                data->statusMessageType = gna::Warning;
                break;
            case 7:
                data->statusMessage = "Ins alignment is complete but vehicle must perform maneuvers so that the attitude can converge";
                data->statusMessageType = gna::Ok;
                break;
            default:
                {
                    stringstream ss;
                    ss <<  "Unknown Ins Status. Check NovatelSpan manual for \"" << insPva.data.insStatus << "\" as INS status";
                    data->statusMessage = ss.str();
                    data->statusMessageType = gna::Warning;
                }
                break;
        }

        return genericData;
    }

    std::auto_ptr<gna::GenericData>
    createExternalMsg(gnua::BestGpsPosLogB &bestGpsPos, struct timeval &timeStamp){
        static int cnt;
        if(0 == cnt++ % 100) cout << __func__ << " gpspos; implement me properly!\n";
        gna::BestGpsPosData *data = new gna::BestGpsPosData;
        std::auto_ptr<gna::GenericData> genericData( data );

        //data
        data->gpsWeekNr = bestGpsPos.header.gpsWeekNr;
        data->msIntoWeek = bestGpsPos.header.msIntoWeek;
        //data->solutionStatus = bestGpsPos.data.solutionStatus;
        //data->positionType = bestGpsPos.data.positionType;
        data->latitude = bestGpsPos.data.latitude;
        data->longitude = bestGpsPos.data.longitude;
        data->heightAMSL = bestGpsPos.data.heightAMSL;
        data->undulation = bestGpsPos.data.undulation;
        data->datumId = bestGpsPos.data.datumId;
        data->sigmaLatitude = bestGpsPos.data.sigmaLatitude;
        data->sigmaLongitude = bestGpsPos.data.sigmaLongitude;
        data->sigmaHeight = bestGpsPos.data.sigmaHeight;
        data->baseStationId[4] = bestGpsPos.data.baseStationId[4];
        data->diffAge = bestGpsPos.data.diffAge;
        data->solutionAge = bestGpsPos.data.solutionAge;
        data->numObservations = bestGpsPos.data.numObservations;
        data->numL1Ranges = bestGpsPos.data.numL1Ranges;
        data->numL1RangesRTK = bestGpsPos.data.numL1RangesRTK;
        data->numL2RangesRTK = bestGpsPos.data.numL2RangesRTK;

        //time
        data->timeStampSec = timeStamp.tv_sec;
        data->timeStampUSec = timeStamp.tv_usec;

        //status
        static bool lastStatusWasGood = false;
        if(true == gnua::receiverStatusIsGood(bestGpsPos.header.receiverStatus)){
            if (true == lastStatusWasGood){
                // still all good, no need to be chatty
                data->statusMessageType = gna::NoMsg;
                data->statusMessage = "";
                lastStatusWasGood = true;
            }else{
                // we are good now, report it
                data->statusMessageType = gna::Ok;
                data->statusMessage = "all is good";
                lastStatusWasGood = true;
            }
        }else{
            //whoops
            data->statusMessageType = gna::Fault; // warning?
            data->statusMessage = gnua::receiverStatusToString(bestGpsPos.header.receiverStatus);
            lastStatusWasGood = false;
        }
        return genericData;

    }

    std::auto_ptr<gna::GenericData>
    createExternalMsg(gnua::BestGpsVelLogB &bestGpsVel, struct timeval &timeStamp){
        static int cnt;
        if(0 == cnt++ % 100) cout << __func__ << " gpsvel; implement me properly!\n";
        gna::BestGpsVelData *data = new gna::BestGpsVelData;
        std::auto_ptr<gna::GenericData> genericData( data );

        //data
        data->gpsWeekNr = bestGpsVel.header.gpsWeekNr;
        data->msIntoWeek = bestGpsVel.header.msIntoWeek;
        //data->solutionStatus = bestGpsVel.data.solutionStatus;
        //data->velocityType = bestGpsVel.data.velocityType;
        data->latency = bestGpsVel.data.latency;
        data->diffAge = bestGpsVel.data.diffAge;
        data->horizontalSpeed = bestGpsVel.data.horizontalSpeed;
        data->trackOverGround = bestGpsVel.data.trackOverGround;
        data->verticalSpeed = bestGpsVel.data.verticalSpeed;

        //time
        data->timeStampSec = timeStamp.tv_sec;
        data->timeStampUSec = timeStamp.tv_usec;

        //status
        static bool lastStatusWasGood = false;
        if(true == gnua::receiverStatusIsGood(bestGpsVel.header.receiverStatus)){
            if (true == lastStatusWasGood){
                // still all good, no need to be chatty
                data->statusMessageType = gna::NoMsg;
                data->statusMessage = "";
                lastStatusWasGood = true;
            }else{
                // we are good now, report it
                data->statusMessageType = gna::Ok;
                data->statusMessage = "all is good";
                lastStatusWasGood = true;
            }
        }else{
            //whoops
            data->statusMessageType = gna::Fault; // warning?
            data->statusMessage = gnua::receiverStatusToString(bestGpsVel.header.receiverStatus);
            lastStatusWasGood = false;
        }
        return genericData;
    }

    std::auto_ptr<gna::GenericData>
    createExternalMsg(gnua::RawImuLogSB &rawImu, struct timeval &timeStamp, gnua::ImuDecoder *imuDecoder){
        //static int cnt;
        //if(0 == cnt++ % 500) cout << __func__ << " imu; implement me properly!\n";
        gna::RawImuData *data = new gna::RawImuData;
        std::auto_ptr<gna::GenericData> genericData( data );

        //data
        data->gpsWeekNr = rawImu.data.gpsWeekNr;
        data->secIntoWeek = rawImu.data.secIntoWeek;
        //accels
        data->zDeltaV = imuDecoder->accelCnt2MperSec(rawImu.data.zAccelCnt);
        data->yDeltaV = -1.0*imuDecoder->accelCnt2MperSec(rawImu.data.yNegativAccelCnt);
        data->xDeltaV = imuDecoder->accelCnt2MperSec(rawImu.data.xAccelCnt);
        //gyros
        data->zDeltaAng = imuDecoder->gyroCnt2Rad(rawImu.data.zGyroCnt);
        data->yDeltaAng = -1.0*imuDecoder->gyroCnt2Rad(rawImu.data.yNegativGyroCnt);
        data->xDeltaAng = imuDecoder->gyroCnt2Rad(rawImu.data.xGyroCnt);

        //time
        data->timeStampSec = timeStamp.tv_sec;
        data->timeStampUSec = timeStamp.tv_usec;

        //status
        static bool lastStatusWasGood = false;
        if(true == imuDecoder->statusIsGood(rawImu.data.imuStatus)){
            if (true == lastStatusWasGood){
                // still all good, no need to be chatty
                data->statusMessageType = gna::NoMsg;
                data->statusMessage = "";
                lastStatusWasGood = true;
            }else{
                // we are good now, report it
                data->statusMessageType = gna::Ok;
                data->statusMessage = "all is good";
                lastStatusWasGood = true;
            }
        }else{
            //whoops
            data->statusMessageType = gna::Fault; // warning?
            data->statusMessage = imuDecoder->statusToString(rawImu.data.imuStatus);
            lastStatusWasGood = false;
        }
        return genericData;
    }

    std::string doubleVectorToString(vector<double > &vec, std::string seperator){
        std::stringstream ss;
        int max = vec.size();
        for (int i=0; i<max; i++){
            ss << vec[i] << seperator;
        }
        return ss.str();
    }
}//namespace
