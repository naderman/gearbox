/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBX_SICK_ACFR_H
#define GBX_SICK_ACFR_H

#include <gbxsickacfr/serialhandler.h>
#include <gbxsickacfr/gbxutilacfr/tracer.h>
#include <gbxsickacfr/gbxutilacfr/status.h>
#include <memory>

namespace gbxsickacfr {

//! Configuration structure
class Config
{   
public:
    Config();
    bool validate() const;
    std::string toString() const;
    bool operator==( const Config & other );
    bool operator!=( const Config & other );

    //! Serial device. e.g. "/dev/ttyS0"
    std::string device;
    //! Baud rate
    int baudRate;
    //! minimum range [m]
    double minRange;
    //! maximum range [m]
    double maxRange;
    //! field of viewe [rad]
    double fieldOfView;
    //! starting angle [rad]
    double startAngle;
    //! number of samples in a scan
    int    numberOfSamples;
};

//! Data structure returned by read()
class Data
{
public:
    Data()
        : haveWarnings(false)
        {}

    float         *ranges;
    unsigned char *intensities;
    int            timeStampSec;
    int            timeStampUsec;
    bool           haveWarnings;
    //! if 'haveWarnings' is set, 'warnings' will contain diagnostic information.
    std::string    warnings;
};

class Driver
{

public: 

    //! Constructor
    Driver( const Config &config, gbxutilacfr::Tracer& tracer, gbxutilacfr::Status& status );

    //! Blocks till new data is available, but shouldn't occupy the thread indefinitely.
    //! Ranges and intensities can be expected to have been pre-sized correctly.
    //! Throws exceptions on un-recoverable faults.
    //!
    //! The default timeout is greater than the scan inter-arrival time for all baudrates.
    void read( Data &data, int timeoutMs=1000 );

private: 

    // Waits up to maxWaitMs for a response of a particular type.
    // Returns true iff it got the response it wanted.
    bool waitForResponseType( uChar type, TimedLmsResponse &response, int maxWaitMs );
    // Returns: true if ack or nack received.
    // (and sets receivedAck: true = ACK, false=NACK)
    bool waitForAckOrNack( bool &receivedAck );

    LmsResponse askLaserForStatusData();
    LmsResponse askLaserForConfigData();

    LmsConfigurationData desiredConfiguration();
    bool isAsDesired( const LmsConfigurationData &lmsConfig );

    int guessLaserBaudRate();

    // Connects to the laser, sets params, and starts continuous mode.
    void initLaser();

    TimedLmsResponse sendAndExpectResponse( const std::vector<uChar> &commandAndData,
                                            bool ignoreErrorConditions=false );

    std::string errorConditions();

    uChar desiredMeasuredValueUnit();
    uint16_t desiredAngularResolution();

    void setBaudRate( int baudRate );

    Config config_;

    std::auto_ptr<SerialHandler> serialHandler_;

    std::vector<uChar> commandAndData_;
    std::vector<uChar> telegramBuffer_;

    gbxutilacfr::Tracer& tracer_;
    gbxutilacfr::Status& status_;
};

} // namespace

#endif
