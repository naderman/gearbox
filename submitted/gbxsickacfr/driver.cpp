/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <cstring>

#include <gbxsickacfr/gbxutilacfr/gbxutilacfr.h>
#include <gbxsickacfr/gbxiceutilacfr/gbxiceutilacfr.h>
#include "driver.h"

using namespace std;

namespace gbxsickacfr {

namespace {

    class NoResponseException : public std::exception
    {
        std::string  message_;
    public:
        NoResponseException(const char *message)
            : message_(message) {}
        NoResponseException(const std::string &message)
            : message_(message) {}
        ~NoResponseException()throw(){}
        virtual const char* what() const throw() { return message_.c_str(); }
    };

    class NackReceivedException : public std::exception
    {
        std::string  message_;
    public:
        NackReceivedException(const char *message)
            : message_(message) {}
        NackReceivedException(const std::string &message)
            : message_(message) {}
        ~NackReceivedException()throw(){}
        virtual const char* what() const throw() { return message_.c_str(); }
    };

    class ResponseIsErrorException : public std::exception
    {
        std::string  message_;
    public:
        ResponseIsErrorException(const char *message)
            : message_(message) {}
        ResponseIsErrorException(const std::string &message)
            : message_(message) {}
        ~ResponseIsErrorException()throw(){}
        virtual const char* what() const throw() { return message_.c_str(); }
    };
}
////////////////////////

Config::Config() :   
    baudRate(38400),
    minRange(0.0),
    maxRange(0.0),
    fieldOfView(0.0),
    startAngle(0.0),
    numberOfSamples(0)
{
}

bool
Config::isValid() const
{
    // Don't bother verifying device, the user will find out soon enough when the Driver bitches.
    if ( !( baudRate == 9600    ||
            baudRate == 19200   ||
            baudRate == 38400   ||
            baudRate == 500000 ) )
    {
        return false;
    }
    if ( minRange < 0.0 ) return false;
    if ( maxRange <= 0.0 ) return false;
    if ( fieldOfView <= 0.0 || fieldOfView > DEG2RAD(360.0) ) return false;
    if ( startAngle <= DEG2RAD(-360.0) || startAngle > DEG2RAD(360.0) ) return false;
    if ( numberOfSamples <= 0 ) return false;

    return true;
}

std::string
Config::toString() const
{
    std::stringstream ss;
    ss << "Laser driver config: device="<<device<<", baudRate="<<baudRate<<", minr="<<minRange<<", maxr="<<maxRange<<", fov="<<RAD2DEG(fieldOfView)<<"deg, start="<<RAD2DEG(startAngle)<<"deg, num="<<numberOfSamples;
    return ss.str();
}

bool 
Config::operator==( const Config & other )
{
    return (minRange==other.minRange && maxRange==other.maxRange && fieldOfView==other.fieldOfView 
         && startAngle==other.startAngle && numberOfSamples==other.numberOfSamples);
}

bool 
Config::operator!=( const Config & other )
{
    return (minRange!=other.minRange || maxRange!=other.maxRange || fieldOfView!=other.fieldOfView 
         || startAngle!=other.startAngle || numberOfSamples!=other.numberOfSamples);
}

////////////////////////

Driver::Driver( const Config &config, gbxutilacfr::Tracer& tracer, gbxutilacfr::Status& status )
    : config_(config),
      tracer_(tracer),
      status_(status)
{
    if ( !config.isValid() )
    {
        stringstream ss;
        ss << __func__ << "(): Invalid config: " << config.toString();
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }

    stringstream ssDebug;
    ssDebug << "Connecting to laser on serial port " << config_.device;
    tracer_.debug( ssDebug.str() );

    serialHandler_.reset( new SerialHandler( config_.device, tracer, status ) );

    try {

        initLaser();

    }
    catch ( const ResponseIsErrorException &e )
    {
        std::string errorLog = errorConditions();
        stringstream ss;
        ss << e.what() << endl << "Laser error log: " << errorLog;
        throw ResponseIsErrorException( ss.str() );
    }
    catch ( const std::exception &e )
    {
        stringstream ss;
        ss << "during initLaser(): " << e.what();
        tracer_.warning( ss.str() );
        throw;
    }
}

bool
Driver::waitForResponseType( uChar type, TimedLmsResponse &response, int maxWaitMs )
{
    gbxiceutilacfr::Timer waitTimer;
    
    while ( true )
    {
        int ret = serialHandler_->getNextResponse( response, maxWaitMs );
        if ( ret == 0 )
        {
            // got response
            if ( response.response.type == type )
                return true;

            // Got a response, but not of the type we were expecting...
            //
            // In general, print out a warning message.
            // However, don't print out a warning if it's an ACK/NACK, because ACK/NACK don't
            // have checksums so they're likely to be found inside a failed-checksum message when a bit
            // gets flipped during transmission.
            if ( response.response.type != ACK &&
                 response.response.type != NACK )
            {
                stringstream ss;
                ss << "Driver::waitForResponseType(): While waiting for " << cmdToString(type) << ", received unexpected response: " << toString(response.response);
                tracer_.warning( ss.str() );
            }

            if ( waitTimer.elapsedMs() > maxWaitMs )
            {
                // waited too long
                return false;
            }
        }
        else if ( ret == -1 )
        {
            // timed out on buffer
            return false;
        }
        else
        {
            stringstream ss; ss << "Weird return code from getAndPopNext: " << ret;
            throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
        }
    }
}

bool
Driver::waitForAckOrNack( bool &receivedAck )
{
    const int maxWaitMs = 1000;

    gbxiceutilacfr::Timer waitTimer;

    while ( true )
    {
        TimedLmsResponse timedResponse;

        int ret = serialHandler_->getNextResponse( timedResponse, maxWaitMs );
        if ( ret == 0 )
        {
            // got timedResponse
            if ( timedResponse.response.type == ACK || timedResponse.response.type == NACK )
            {
                receivedAck = (timedResponse.response.type == ACK);
                return true;
            }
            else
            {
                stringstream ss;
                ss << "Driver::waitForAckOrNack(): Received unexpected response: " << toString(timedResponse.response);
                tracer_.warning( ss.str() );
            }

            double waitTimeSec = waitTimer.elapsedSec();
            if ( waitTimeSec > maxWaitMs/1000.0 )
            {
                return false;
            }
        }
        else if ( ret == -1 )
        {
            tracer_.debug( "Driver: waitForAckOrNack(): timed out.", 3 );
            return false;
        }
        else
        {
            stringstream ss;
            ss << "Weird return code from serialHandler_->getNextResponse: " << ret;
            throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
        }
    }    
}

LmsResponse
Driver::askLaserForStatusData()
{
    constructStatusRequest( commandAndData_ );

    TimedLmsResponse response = sendAndExpectResponse( commandAndData_ );
    return response.response;
}

LmsResponse
Driver::askLaserForConfigData()
{
    constructConfigurationRequest( commandAndData_ );
    try {
        TimedLmsResponse response = sendAndExpectResponse( commandAndData_ );
        return response.response;
    }
    catch ( NoResponseException &e )
    {
        stringstream ss;
        ss << "While trying to askLaserForConfigData(): " << e.what();
        throw NoResponseException( ss.str() );
    }
}

uChar
Driver::desiredMeasuredValueUnit()
{
    if ( (int)(round(config_.maxRange)) == 80 )
    {
        return MEASURED_VALUE_UNIT_CM;
    }
    else if ( (int)(round(config_.maxRange)) == 8 )
    {
        return MEASURED_VALUE_UNIT_MM;
    }
    else
    {
        stringstream ss;
        ss << "Unknown linear resolution: " << config_.maxRange;
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }
}

uint16_t
Driver::desiredAngularResolution()
{
    double angleIncrement = config_.fieldOfView / (config_.numberOfSamples-1);
    int angleIncrementInHundredthDegrees = (int)round(100.0 * angleIncrement*180.0/M_PI);

    assert( angleIncrementInHundredthDegrees == ANGULAR_RESOLUTION_1_0_DEG ||
            angleIncrementInHundredthDegrees == ANGULAR_RESOLUTION_0_5_DEG ||
            angleIncrementInHundredthDegrees == ANGULAR_RESOLUTION_0_25_DEG );
    return angleIncrementInHundredthDegrees;
}

bool 
Driver::isAsDesired( const LmsConfigurationData &lmsConfig )
{
    // This thing has a default constructor which will fill out most things
    return ( lmsConfig == desiredConfiguration() );
}

LmsConfigurationData
Driver::desiredConfiguration()
{
    // Default constructor sets up reasonable defaults which we then modify
    LmsConfigurationData c;
    
    c.measuringMode = MEASURING_MODE_8m80m_REFLECTOR8LEVELS;
    c.measuredValueUnit = desiredMeasuredValueUnit();

    // AlexB: It's not entirely clear, but from reading the SICK manual
    //        it looks like perhaps setting availability to 0x01 may
    //        help with dazzle (ie sunlight interfering with the laser).
    //        I haven't had a chance to test it though, so I'm not game
    //        to set it.
    cout<<"TRACE(driver.cpp): TODO: set availability?" << endl;
    // c.availability = 0x01;

    return c;
}

std::string
Driver::errorConditions()
{
    try {
        constructRequestErrorMessage( commandAndData_ );
        const bool ignoreErrorConditions = true;
        TimedLmsResponse errorResponse = sendAndExpectResponse( commandAndData_, ignoreErrorConditions );
        return toString( errorResponse.response );
    }
    catch ( std::exception &e )
    {
        stringstream ss;
        ss << "Driver: Caught exception while getting error conditions: " << e.what();
        tracer_.debug( ss.str() );
        throw;
    }
}

void
Driver::setBaudRate( int baudRate )
{
    constructRequestBaudRate( commandAndData_, baudRate );
    sendAndExpectResponse( commandAndData_ );
    serialHandler_->setBaudRate( config_.baudRate );
}

int
Driver::guessLaserBaudRate()
{
    // Guess our current configuration first: maybe the laser driver was re-started.
    std::vector<int> baudRates;
    baudRates.push_back( config_.baudRate );
    if ( config_.baudRate != 9600 ) baudRates.push_back( 9600 );
    if ( config_.baudRate != 19200 ) baudRates.push_back( 19200 );
    if ( config_.baudRate != 38400 ) baudRates.push_back( 38400 );
    if ( config_.baudRate != 500000 ) baudRates.push_back( 500000 );
    
    for ( uint baudRateI=0; baudRateI < baudRates.size(); baudRateI++ )
    {
        stringstream ss;
        ss << "Driver: Trying to connect at " << baudRates[baudRateI] << " baud.";
        tracer_.info( ss.str() );

        serialHandler_->setBaudRate( baudRates[baudRateI] );
            
        const uint MAX_TRIES = 2;
        for ( uint tryNum=0; tryNum < MAX_TRIES; tryNum++ )
        {
            try {
                stringstream ss;
                ss <<"Driver: Trying to get laser status with baudrate " << baudRates[baudRateI] << " (try number "<<tryNum<<" of " << MAX_TRIES << ")" << endl;
                tracer_.debug( ss.str() );
                askLaserForStatusData();
                return baudRates[baudRateI];
            }
            catch ( const NoResponseException &e )
            {
                stringstream ss;
                ss << "Driver::guessLaserBaudRate(): try " << tryNum << " of " << MAX_TRIES << "  at baudRate " << baudRates[baudRateI] << " failed: " << e.what();
                tracer_.debug( ss.str() );
                if ( tryNum == MAX_TRIES-1 )
                    break;
            }
        }
    } // end loop over baud rates

    throw gbxutilacfr::Exception( ERROR_INFO, "Failed to detect laser baud rate." );
}

void
Driver::initLaser()
{
    int currentBaudRate = guessLaserBaudRate();
    tracer_.debug( "Driver: Guessed the baud rate!" );

    //
    // Turn continuous mode off
    //
    tracer_.debug("Driver: Turning continuous mode off");
    // For some reason this isn't always reliable, not too sure why.
    // Perhaps there's some crap left in the buffer after the thing
    // was previously in continuous mode?
    const int MAX_TRIES=3;
    for ( uint i=0; i < MAX_TRIES; i++ )
    {
        try {
            constructRequestMeasuredOnRequestMode( commandAndData_ );
            sendAndExpectResponse( commandAndData_ );
            break;
        }
        catch ( NoResponseException &e )
        {
            if ( i == MAX_TRIES-1 )
            {
                // Give up
                throw;
            }
        }
    }

    //
    // Set Desired BaudRate
    //
    if ( currentBaudRate != config_.baudRate )
    {
        setBaudRate( config_.baudRate );
    }

    // Gather info about the SICK
    stringstream ssInfo;
    ssInfo << "Laser info prior to initialisation:" << endl;

    //
    // Make note of error log
    //
    ssInfo << errorConditions() << endl;

    //
    // Check operating data counters
    //
    constructRequestOperatingDataCounter( commandAndData_ );
    TimedLmsResponse counterResponse = sendAndExpectResponse( commandAndData_ );
    ssInfo << "OperatingDataCounter: " << toString(counterResponse.response) << endl;

    //
    // Get status
    //
    LmsResponse statusResponse = askLaserForStatusData();
    LmsStatusResponseData *statusResponseData = 
        dynamic_cast<LmsStatusResponseData*>(statusResponse.data);
    assert( statusResponseData != NULL );
    ssInfo << "Status: " << statusResponseData->toString() << endl;

    LmsResponse configResponse = askLaserForConfigData();
    LmsConfigurationData *lmsConfig = 
        dynamic_cast<LmsConfigurationData*>(configResponse.data);
    assert( lmsConfig != NULL );
    ssInfo << "Config: " << configResponse.data->toString() << endl;

    tracer_.info( ssInfo.str() );

    //
    // Enter installation mode
    //
    constructRequestInstallationMode( commandAndData_ );
    sendAndExpectResponse( commandAndData_ );

    //
    // Configure the thing if we have to
    //
    if ( !isAsDesired( *lmsConfig ) )
    {
        tracer_.info( "Driver: Have to reconfigure the laser..." );

        constructConfigurationCommand( desiredConfiguration(),
                                       commandAndData_ );
        TimedLmsResponse configCmdResponse = sendAndExpectResponse( commandAndData_ );
        LmsConfigurationResponseData *configCmdResponseData = 
            dynamic_cast<LmsConfigurationResponseData*>(configCmdResponse.response.data);
        if ( !isAsDesired( configCmdResponseData->config ) )
        {
            stringstream ss;
            ss << "Error configuring SICK:  Config after configuration not what we expect.  Asked for: " << desiredConfiguration().toString() << endl << "got: " << configCmdResponseData->toString();
            throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
        }
    }

    //
    // Configure the angular resolution
    //
    const uint16_t desiredScanningAngle = 180;
    constructSwitchVariant( desiredScanningAngle,
                            desiredAngularResolution(),
                            commandAndData_ );
    TimedLmsResponse angResponse = sendAndExpectResponse( commandAndData_ );
    LmsSwitchVariantResponseData *angResponseData =
        dynamic_cast<LmsSwitchVariantResponseData*>(angResponse.response.data);
    assert( angResponseData != NULL );
    if ( !( angResponseData->scanningAngle == desiredScanningAngle &&
            angResponseData->angularResolution == desiredAngularResolution() ) )
    {
            stringstream ss;
            ss << "Error configuring SICK variant:  Variant after configuration not what we expect: " << angResponseData->toString();
            throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );        
    }
    
    //
    // Start continuous mode
    //
    constructRequestContinuousMode( commandAndData_ );
    TimedLmsResponse contResponse = sendAndExpectResponse( commandAndData_ );

    tracer_.info( "Driver: enabled continuous mode, laser is running." );
}

TimedLmsResponse
Driver::sendAndExpectResponse( const std::vector<uChar> &commandAndData, bool ignoreErrorConditions )
{
    constructTelegram( telegramBuffer_, commandAndData );

    assert( commandAndData.size()>0 && "constructTelegram should give telegram with non-zero size." );
    const uChar command = commandAndData[0];

    stringstream ss;
    ss << "Driver: requesting send "<<cmdToString(command)<<endl<<"  telegram: " 
       << toHexString(telegramBuffer_);
    tracer_.debug( ss.str(), 3 );

    serialHandler_->send( telegramBuffer_ );

    bool isAck;
    bool receivedAckOrNack = waitForAckOrNack( isAck );
    if ( !receivedAckOrNack )
    {
        throw NoResponseException( "No ACK/NACK to command "+cmdToString(command) );
    }
    if ( !isAck )
    {
        throw NackReceivedException( "Received NACK for command "+cmdToString(command) );
    }

    int timeoutMs = 1000;
    if ( command == CMD_CONFIGURE_LMS )
    {
        // This takes a long time to complete
        timeoutMs = 15000;
    }

    TimedLmsResponse response;
    bool received = waitForResponseType( ack(command), response, timeoutMs );
    if ( !received )
    {
        throw NoResponseException( "No response to command "+cmdToString(command) );
    }

    if ( !ignoreErrorConditions && response.response.isError() )
    {
        throw ResponseIsErrorException( "Response contains an error condition: "+toString(response.response) );
    }
    if ( response.response.isWarn() )
    {
        stringstream ss;
        tracer_.warning( "Response contains warning: "+toString(response.response) );
    }

    return response;
}

void 
Driver::read( Data &data )
{
    TimedLmsResponse response;

    // This timeout is greater than the scan inter-arrival time for all baudrates.
    const int timeoutMs = 1000;
    bool received = waitForResponseType( ACK_REQUEST_MEASURED_VALUES, response, timeoutMs );
    if ( !received )
    {
        throw gbxutilacfr::Exception( ERROR_INFO, "No scan received." );
    }


    LmsMeasurementData *measuredData = (LmsMeasurementData*)response.response.data;

    if ( response.response.isError() )
    {
        std::string errorLog = errorConditions();
        stringstream ss;
        ss << "Scan data indicates errors: " << toString(response.response) << endl << "Laser error log: " << errorLog;
        throw ResponseIsErrorException( ss.str() );        
    }
    else if ( response.response.isWarn() )
    {
        data.haveWarnings = true;
        stringstream ss;
        ss << "Scan data indicates warnings: " << toString(response.response);
        data.warnings = ss.str();
    }

    memcpy( &(data.ranges[0]), &(measuredData->ranges[0]), measuredData->ranges.size()*sizeof(float) );
    memcpy( &(data.intensities[0]), &(measuredData->intensities[0]), measuredData->intensities.size()*sizeof(unsigned char) );
    data.timeStampSec = response.timeStampSec;
    data.timeStampUsec = response.timeStampUsec;
}

} // namespace
