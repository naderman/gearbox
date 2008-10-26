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
    : gbxiceutilacfr::SafeThread( tracer ),
      serial_(serialPort),
      responseParser_(responseParser),
      responseBuffer_(-1,gbxiceutilacfr::BufferTypeCircular),
      unparsedBytesWarnThreshold_(unparsedBytesWarnThreshold),
      tracer_(tracer),
      subStatus_( status, subsysName )
{
    subStatus_.setMaxHeartbeatInterval( 60 );
}

SerialDeviceHandler::~SerialDeviceHandler()
{
    //
    // The component may outlive this subsystem.
    // So tell status that it might not hear from us for a while.
    //
    subStatus_.setMaxHeartbeatInterval( 1e9 );
}

void
SerialDeviceHandler::setBaudRate( int baudRate )
{
    tracer_.debug( "SerialDeviceHandler: Changing baud rate of serial port." );
    serial_.setBaudRate( baudRate );
    // TODO: AlexB: not entirely sure if these are
    // necessary, they should either be removed or
    // added to the setBaudRate function.
    serial_.flush();
    serial_.drain();
}

void
SerialDeviceHandler::send( const char *commandBytes, int numCommandBytes )
{
    serial_.write( commandBytes, numCommandBytes );
}

void
SerialDeviceHandler::walk()
{
    double maxIntervalSec = serial_.timeout().sec + 1e6*serial_.timeout().usec;
    subStatus_.setMaxHeartbeatInterval( maxIntervalSec * 5.0 );

    while ( !isStopping() )
    {
        if ( SUPER_DEBUG )
            tracer_.debug( "SerialDeviceHandler::walk() start of loop.", 6 );

        try {

            // Wait for data to arrive, put it in our buffer_
            try {
                if ( getDataFromSerial() )
                {
                    // Process it
                    try {
                        bool statusOK = processBuffer();
                        if ( statusOK )
                            subStatus_.ok();
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
                    subStatus_.ok();
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
            subStatus_.fault( ss.str() );
        }
        catch ( std::exception &e )
        {
            stringstream ss;
            ss << "SerialDeviceHandler: Caught exception: " << e.what();
            subStatus_.fault( ss.str() );
        }
        catch ( ... )
        {
            stringstream ss;
            ss << "SerialDeviceHandler: Caught unknown exception.";
            subStatus_.fault( ss.str() );
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
        ss << "SerialDeviceHandler::getDataFromSerial(): nBytes available: " << nBytes;
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
SerialDeviceHandler::processBuffer()
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
        IceUtil::Time t = IceUtil::Time::now();
        int timeStampSec = (int)t.toSeconds();
        int timeStampUsec = (int)t.toMicroSeconds() - timeStampSec*1000000;        

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
/* ALEXM: this is the diff from post-bindoon which broke poly device
        if ( numBytesParsed == 0 )
        {
            stringstream ss;
            ss << "SerialDeviceHandler: Zero bytes were parsed, but buffer contains "<<buffer_.size()<<" bytes! (gotMessage="<<gotMessage<<").  responseParser_ shouldn't do this to us.";
            tracer_.warning( ss.str() );
            numBytesParsed = 1;
        }
ALEXM */
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
                subStatus_.warning( ss.str() );
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
