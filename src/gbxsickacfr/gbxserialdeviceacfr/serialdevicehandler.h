/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#ifndef GBXSERIALDEVICEACFR_SERIALDEVICEHANDLER_H
#define GBXSERIALDEVICEACFR_SERIALDEVICEHANDLER_H

#include <gbxserialacfr/serial.h>
#include <gbxutilacfr/substatus.h>
#include <gbxsickacfr/gbxiceutilacfr/safethread.h>
#include <gbxsickacfr/gbxiceutilacfr/buffer.h>
#include <IceUtil/IceUtil.h>

namespace gbxserialdeviceacfr {

//! @brief A generic Response: a message received from the device
class IResponse : public IceUtil::Shared
{
public:
    ~IResponse() {}
    
    // Does the response indicate a warning condition?
    // If so, this will be reported to SerialDeviceHandler's status.
    virtual bool isWarn() const=0;
    // Does the response indicate an error condition?
    // If so, this will be reported to SerialDeviceHandler's status.
    virtual bool isError() const=0;
    
    // Human-readable string
    virtual std::string toString() const=0;
};
typedef IceUtil::Handle<IResponse> IResponsePtr;

//! Response plus a timeStamp: a simple container to keep the two together
class TimedResponse {
public:

    // Require an empty constructor to put in a buffer
    TimedResponse() {}
    TimedResponse( int s, int us, const IResponsePtr &r )
        : timeStampSec(s), timeStampUsec(us), response(r) {}

    int timeStampSec;
    int timeStampUsec;
    IResponsePtr response;
};

//!
//! The implementation of this class needs to be provided by the user.
//! It parses buffers to produce discrete messages (responses form the device)
//!
class IResponseParser {

public:

    virtual ~IResponseParser() {}

    // Parses the contents of the buffer.
    // Params:
    //   - 'response':       the parsed response
    //   - 'numBytesParsed': this function will set this to the number of bytes parsed
    //                       (irrespective of whether or not a valid response was found)
    // Returns:
    //   - true:  a valid response was found.
    //   - false: no valid response found, but we still might have parsed (and thrown out) some bytes.
    virtual bool parseBuffer( const std::vector<char> &buffer,
                              IResponsePtr  &response,
                              int           &numBytesParsed )=0;

};

//!
//! @brief Handles the serial port.
//!
//! This thread waits for new messags to arrive from the device, parses
//! them and sticks them into a buffer for someone else to grab. 
//!
//! Read in this separate loop so we can hopefully grab the messages
//! as soon as they arrive, without relying on an external poller
//! which may be busy doing other stuff.
//! This will hopefully give us accurate timestamps.
//!
//! @author Alex Brooks
//!
class SerialDeviceHandler : public gbxiceutilacfr::SafeThread
{

public: 

    // Params:
    //   - subsysName: given to Status
    //   - unparsedBytesWarnThreshold: if we get more than this many un-parsed bytes packed into the
    //                                 receive buffer, flag a warning.
    //   - serialPort_: must have timeouts enabled
    SerialDeviceHandler( const std::string     &subsysName,
                         gbxserialacfr::Serial &serialPort,
                         IResponseParser       &responseParser,
                         gbxutilacfr::Tracer   &tracer,
                         gbxutilacfr::Status   &status,
                         int                    unparsedBytesWarnThreshold = 20000 );

    ~SerialDeviceHandler();

    // Send the bytes to the device
    void send( const char* commandBytes, int numCommandBytes );

    // allows changing of baud rates on-the-fly
    void setBaudRate( int baudRate );

    // The main thread function, inherited from SubsystemThread
    virtual void walk();

    // Allow external non-const access direct to (thread-safe) responseBuffer.
    // This is what you use to hear responses.
    gbxiceutilacfr::Buffer<TimedResponse> &responseBuffer() { return responseBuffer_; }

private: 

    // Returns: true if got data, false if timed out
    bool getDataFromSerial();
    // Returns: true if statusOK, false it something bad happened
    bool processBuffer();

    gbxserialacfr::Serial &serial_;

    // Knows how to parse for responses
    IResponseParser &responseParser_;

    // Contains un-parsed data from the device
    std::vector<char> buffer_;

    // Thread-safe store of responses from the device
    gbxiceutilacfr::Buffer<TimedResponse> responseBuffer_;

    int unparsedBytesWarnThreshold_;

    gbxutilacfr::Tracer& tracer_;
    gbxutilacfr::SubStatus subStatus_;
};
//! A smart pointer to the class.
typedef IceUtil::Handle<SerialDeviceHandler> SerialDeviceHandlerPtr;

//////////////////////////////////////////////////////////////////////
// Printing Functions
//////////////////////////////////////////////////////////////////////

std::string toHexString( const char *buf, int bufLen );
// inline std::string toHexString( char *buf, int bufLen )
// { return toHexString( (const unsigned char *)buf, bufLen ); }
inline std::string toHexString( const std::vector<char> &buf )
{return toHexString( &(buf[0]), buf.size() );}

} // namespace

#endif
