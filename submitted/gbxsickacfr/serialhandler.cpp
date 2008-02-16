/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#include "serialhandler.h"
#include <iostream>

using namespace std;

namespace gbxsickacfr {

SerialHandler::SerialHandler( const std::string &dev,
                    gbxutilacfr::Tracer       &tracer,
                    gbxutilacfr::Status       &status )
    : serialPort_( dev.c_str(), 9600, true ),
      serialDeviceHandler_( 
            new gbxserialdeviceacfr::SerialDeviceHandler( "LaserSerialHandler",
                                            serialPort_, responseParser_, tracer, status ) ),
      serialDeviceHandlerThreadPtr_( serialDeviceHandler_ )
{
    serialDeviceHandler_->start();
}

SerialHandler::~SerialHandler()
{
    gbxiceutilacfr::stopAndJoin( serialDeviceHandlerThreadPtr_ );
}

}
