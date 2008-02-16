/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#ifndef SICK_ACFR_SERIALHANDLER_H
#define SICK_ACFR_SERIALHANDLER_H

#include <gbxsickacfr/messages.h>
#include <gbxserialacfr/serial.h>
#include <gbxsickacfr/gbxiceutilacfr/thread.h>
#include <gbxsickacfr/gbxiceutilacfr/buffer.h>
#include <gbxsickacfr/gbxserialdeviceacfr/serialdevicehandler.h>

namespace gbxsickacfr {

class ResponseParser : public gbxserialdeviceacfr::IResponseParser 
{
public:
    bool parseBuffer( const std::vector<char>         &buffer,
                      gbxserialdeviceacfr::IResponsePtr &response,
                      int                             &numBytesParsed )
        {
            LmsResponse *lmsResponse;
            int gotResponse = parseBufferForResponses( (const uChar*)&(buffer[0]),
                                                       buffer.size(),
                                                       lmsResponse,
                                                       numBytesParsed );
            if ( gotResponse )
            {
                response = lmsResponse;
            }
            return gotResponse;
        }

};

// LmsResponse plus a timeStamp
class TimedLmsResponse {
public:
    TimedLmsResponse() {}
    TimedLmsResponse( int s, int us, const LmsResponse &r )
        : timeStampSec(s), timeStampUsec(us), response(r) {}

    int timeStampSec;
    int timeStampUsec;
    LmsResponse response;
};

//
// @brief Handles the serial port.
//
// Read in this separate loop so we can hopefully grab the messages
// as soon as they arrive, without relying on having Driver::read()
// called by an external thread which may be doing other stuff.
// This will hopefully give us more accurate timestamps.
//
// @author Alex Brooks
//
class SerialHandler
{

public: 

    SerialHandler( const std::string &dev,
                    gbxutilacfr::Tracer       &tracer,
                    gbxutilacfr::Status       &status );
    ~SerialHandler();

    void send( const std::vector<uChar> &telegram )
        { serialDeviceHandler_->send( (const char*)&(telegram[0]), telegram.size() ); }

    void setBaudRate( int baudRate )
        { serialDeviceHandler_->setBaudRate( baudRate ); }

    // waits up to maxWaitMs for a Response
    // return codes same as gbxiceutilacfr::Buffer
    int getNextResponse( TimedLmsResponse &timedResponse, int maxWaitMs )
        { 
            gbxserialdeviceacfr::TimedResponse genericTimedResponse;
            int ret = serialDeviceHandler_->responseBuffer().getAndPopNext( genericTimedResponse, maxWaitMs );
            if ( ret == 0 )
            {
                timedResponse.timeStampSec = genericTimedResponse.timeStampSec;
                timedResponse.timeStampUsec = genericTimedResponse.timeStampUsec;

                // This cast is safe becuase the response had to have been generated by ResponseParser
                LmsResponse *lmsResponse = (LmsResponse*) &(*(genericTimedResponse.response));

                timedResponse.response = *lmsResponse;
            }
            return ret;
        }

private: 

    ResponseParser                          responseParser_;
    gbxserialacfr::Serial                     serialPort_;
    gbxserialdeviceacfr::SerialDeviceHandler *serialDeviceHandler_;
    // Keep a smart pointer to the SerialDeviceHandler as a thread, for stop/start purposes
    gbxiceutilacfr::ThreadPtr                 serialDeviceHandlerThreadPtr_;
};

}

#endif
