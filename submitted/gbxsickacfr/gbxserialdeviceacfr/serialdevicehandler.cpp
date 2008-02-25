/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#include "serialdevicehandler.h"
#include <iostream>
#include <iomanip>
#include <cstring>

using namespace std;

namespace gbxsickacfr {
namespace gbxserialdeviceacfr {

namespace {

    const bool SUPER_DEBUG = false;

    // Clear out the bytes we've just parsed, and shift the un-parsed data
    // to the front of the buffer.
    void removeParsedData( std::vector<char> &buffer,
                           int numBytesParsed )
    {
        if ( numBytesParsed > 0 )
        {
            int numBytesBeyond = buffer.size()-numBytesParsed;
            if ( numBytesBeyond > 0 )
            {
                memmove( &(buffer[0]), &(buffer[numBytesParsed]), numBytesBeyond*sizeof(char) );
            }
            buffer.resize( numBytesBeyond );
        }
    }

}

//////////////////////////////////////////////////////////////////////

SerialDeviceHandler::SerialDeviceHandler( const std::string     &subsysName,
                                          gbxserialacfr::Serial &serialPort,
                                          IResponseParser       &responseParser,
                                          gbxutilacfr::Tracer   &tracer,
                                          gbxutilacfr::Status   &status,
                                          int                    unparsedBytesWarnThreshold )
    : gbxiceutilacfr::SubsystemThread( tracer, status, subsysName ),
      serial_(serialPort),
      responseParser_(responseParser),
      responseBuffer_(-1,gbxiceutilacfr::BufferTypeCircular),
      isMessageWaitingToBeSent_(false),
      baudRateChangePending_(false),
      unparsedBytesWarnThreshold_(unparsedBytesWarnThreshold),
      tracer_(tracer),
      status_(status)
{
    status_.setMaxHeartbeatInterval( subsysName, 60 );
    status_.initialising( subsysName );
}

SerialDeviceHandler::~SerialDeviceHandler()
{
    //
    // The component may outlive this subsystem.
    // So tell status that it might not hear from us for a while.
    //
    status_.setMaxHeartbeatInterval( subsysName(), 1e9 );
}

void
SerialDeviceHandler::setBaudRate( int baudRate )
{
    stringstream ss; ss << "SerialDeviceHandler: baud rate change requested: " << baudRate;
    tracer_.debug( ss.str() );

    IceUtil::Mutex::Lock lock(mutex_);

    assert( !baudRateChangePending_ );

    baudRateChangePending_ = true;
    newBaudRate_ = baudRate;
}

void
SerialDeviceHandler::send( const char *commandBytes, int numCommandBytes )
{
    IceUtil::Mutex::Lock lock(mutex_);

    if ( isMessageWaitingToBeSent_ )
    {
        stringstream ss;
        ss << "SerialDeviceHandler::send(): there's a message already waiting to be sent!";
        throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
    }
    isMessageWaitingToBeSent_ = true;
    toSend_.resize(numCommandBytes);
    memcpy( &(toSend_[0]), commandBytes, numCommandBytes*sizeof(char) );
}

void
SerialDeviceHandler::walk()
{
    status_.setMaxHeartbeatInterval( subsysName(), 2 );

    while ( !isStopping() )
    {
        if ( SUPER_DEBUG )
            tracer_.debug( "SerialDeviceHandler::walk() start of loop.", 6 );

        try {

            // Check for house-keeping jobs first
            try
            {
                IceUtil::Mutex::Lock lock(mutex_);

                if ( baudRateChangePending_ )
                {
                    tracer_.debug( "SerialDeviceHandler: Changing baud rate and flushing." );
                    baudRateChangePending_ = false;
                    serial_.setBaudRate( newBaudRate_ );
                    // TODO: AlexB: not entirely sure if these are
                    // necessary, they should either be removed or
                    // added to the setBaudRate function.
                    serial_.flush();
                    serial_.drain();
                }

                if ( isMessageWaitingToBeSent_ )
                {
                    stringstream ss;
                    ss<<"SerialDeviceHandler: sending: " << toHexString(toSend_);
                    tracer_.debug( ss.str() );

                    isMessageWaitingToBeSent_ = false;
                    serial_.write( &(toSend_[0]), toSend_.size() );
                }
            }
            catch ( std::exception &e )
            {
                stringstream ss;
                ss << "SerialDeviceHandler: During house-keeping jobs: " << e.what();
                tracer_.error( ss.str() );
                throw;
            }

            // Wait for data to arrive, put it in our buffer_
            try {
                if ( getDataFromSerial() )
                {
                    IceUtil::Time t = IceUtil::Time::now();
                    int timeStampSec = (int)t.toSeconds();
                    int timeStampUsec = (int)t.toMicroSeconds() - timeStampSec*1000000;
                    // Process it
                    try {
                        bool statusOK = processBuffer( timeStampSec, timeStampUsec );
                        if ( statusOK )
                            status_.ok( subsysName() );
                    }
                    catch ( std::exception &e )
                    {
                        stringstream ss;
                        ss << "SerialDeviceHandler: During processBuffer: " << e.what();
                        tracer_.error( ss.str() );
                        throw;
                    }
                }
                else
                {
                    status_.ok( subsysName() );
                }
            }
            catch ( std::exception &e )
            {
                stringstream ss;
                ss << "SerialDeviceHandler: while reading from serial: " << e.what();
                tracer_.error( ss.str() );
                throw;
            }
        }
        catch ( IceUtil::Exception &e )
        {
            stringstream ss;
            ss << "SerialDeviceHandler: Caught Ice exception: " << e;
            status_.fault( subsysName(), ss.str() );
        }
        catch ( std::exception &e )
        {
            stringstream ss;
            ss << "SerialDeviceHandler: Caught exception: " << e.what();
            status_.fault( subsysName(), ss.str() );
        }
        catch ( ... )
        {
            stringstream ss;
            ss << "SerialDeviceHandler: Caught unknown exception.";
            status_.fault( subsysName(), ss.str() );
        }
    }
}

bool
SerialDeviceHandler::getDataFromSerial()
{
    int nBytes = serial_.bytesAvailableWait();

    if ( SUPER_DEBUG )
    {
        stringstream ss;
        ss << "SerialDeviceHandler::getDataFromSerial(): nBytes: " << nBytes;
        tracer_.debug( ss.str(), 9 );
    }

    if ( nBytes > 0 )
    {
        // Resize buffer to make room for new data
        buffer_.resize( buffer_.size() + nBytes );
        int readStart = buffer_.size()-nBytes;
        int numRead = serial_.read( &(buffer_[readStart]), nBytes );
        assert( (numRead == nBytes) && "serial_.read should read exactly the number we ask for." );
        
        if ( (int)(buffer_.size()) > unparsedBytesWarnThreshold_ )
        {
            stringstream ss;
            ss << "SerialDeviceHandler:: Buffer is getting pretty big -- size is " << buffer_.size();
            tracer_.warning( ss.str() );
        }

        return true;
    }
    return false;
}

bool
SerialDeviceHandler::processBuffer( const int &timeStampSec, int &timeStampUsec )
{
    if ( SUPER_DEBUG )
    {
        stringstream ssDebug;
        ssDebug << "SerialDeviceHandler::processSerialBuffer: buffer is: " << toHexString(buffer_);
        tracer_.debug( ssDebug.str() );
    }

    bool statusOK = true;

    // This loop is in case multiple messages arrived.
    while ( true )
    {
        IResponsePtr response;
        int numBytesParsed = 0;
        bool gotMessage = false;
        try {
            gotMessage = responseParser_.parseBuffer( buffer_,
                                                      response,
                                                      numBytesParsed );
        }
        catch ( const IceUtil::Exception &e )
        {
            stringstream ss;
            ss << "SerialDeviceHandler: While parsing buffer for responses: " << e;
            tracer_.warning( ss.str() );
            throw;
        }
        catch ( const std::exception &e )
        {
            stringstream ss;
            ss << "SerialDeviceHandler: While parsing buffer for responses: " << e.what();
            tracer_.warning( ss.str() );
            throw;
        }

        removeParsedData( buffer_, numBytesParsed );

        if ( gotMessage )
        {
            if ( response == 0 )
            {
                stringstream ss;
                ss << "SerialDeviceHandler::processBuffer(): responseParser said it got a message"
                   << endl << "but response pointer is NULL.";
                throw gbxutilacfr::Exception( ERROR_INFO, ss.str() );
            }

            responseBuffer_.push( TimedResponse( timeStampSec, timeStampUsec, response ) );

            if ( response->isError() || response->isWarn() )
            {
                stringstream ss;
                ss << "SerialDeviceHandler: Received abnormal response: " << response->toString();
                status_.warning( subsysName(), ss.str() );
                statusOK = false;
            }
            else
            {
                statusOK = true;
            }
        }
        else
        {
            break;
        }
    }
    return statusOK;
}

//////////////////////////////////////////////////////////////////////
// Printing Functions
//////////////////////////////////////////////////////////////////////

std::string 
toHexString( const char *buf, int bufLen )
{
    stringstream ss;
    ss << "[ ";
    for ( int i=0; i < bufLen; i++ )
    {
        ss <<hex<<std::setfill('0')<<std::setw(2)<<(int)((unsigned char)buf[i])<<" ";
    }
    ss << "]";
    return ss.str();
}

}
} // namespace
