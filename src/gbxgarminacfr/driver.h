/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Duncan Mercer, Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXGARMINACFR_DRIVER_H
#define GBXGARMINACFR_DRIVER_H

#include <gbxserialacfr/serial.h>
#include <gbxutilacfr/tracer.h>
#include <gbxutilacfr/status.h>
#include <memory>

namespace gbxgarminacfr {

//! Configuration structure
class Config
{   
public:
    Config() :
        readGga(true),
        readVtg(true),
        readRme(true),
        ignoreUnknown(false) {};

    //! Returns true if the configuration is sane. Checks include:
    //! - a non-empty device name
    //! - at least one read-sentence flag set to TRUE
    bool isValid() const;

    //! Returns human-readable configuration description.
    std::string toString() const;

    //! Serial device. e.g. "/dev/ttyS0"
    std::string device;

    //! Read GPGGA sentence
    bool readGga;
    //! Read GPVTG sentence
    bool readVtg;
    //! Read PGRME sentence
    bool readRme;

    //! Ignore unknown messages. This driver tries to turn off all messages and then explicitely enables
    //! the ones it understands. But with some devices this does not work and many messages types are received. 
    //! When ignoreUnknown is set to TRUE the driver quietly ignores the messages it does not understand.
    bool ignoreUnknown;
};

//! Possible types GenericData can contain
enum DataType 
{
    //! Contents of PGGGA message.
    GpGga,
    //! Contents of PGVTG message.
    GpVtg,
    //! Contents of PGRME message.
    PgRme
};

//! Generic data type returned by a read
class GenericData
{
public:
    virtual ~GenericData() {};
    //! Returns data type.
    virtual DataType type() const=0;

private:
};

//! GPS fix types.
enum FixType 
{
    //! Invalid or not available
    Invalid,
    //! Autonomous position
    //! (This is the normal case for non-differential GPS)
    Autonomous,
    //! Differentially corrected
    Differential
};

//! Fix data structure. Note that when fixType is Invalid, all other data except the time stamps
//! are meaningless.
struct GgaData : public GenericData
{
public:
    DataType type() const { return GpGga; }

    //! Time (according to the computer clock) when data was measured.
    //! Number of seconds
    int timeStampSec;
    //! Time (according to the computer clock) when data was measured.
    //! Number of microseconds
    int timeStampUsec;

    //! UTC time (according to the GPS device), reference is Greenwich.
    //! Hour [0..23]
    int utcTimeHrs;
    //! UTC time (according to the GPS device), reference is Greenwich.
    //! Minutes [0..59]
    int utcTimeMin;
    //! UTC time (according to the GPS device), reference is Greenwich.
    //! Seconds [0.0..59.9999(9)]
    double utcTimeSec;

    //! Latitude [degrees]
    double latitude;
    //! Longitude [degrees]
    double longitude;
    //! Altitude [metres above ellipsoid]
    double altitude;
    
    //! Fix type. When fixType is Invalid, all other data except the time stamps
    //! are meaningless.
    FixType fixType;

    //! Number of satellites
    int satellites;

    //! Horizontal dilution of position [metres]
    double horizontalDilutionOfPosition;
    
    //! Height of geoid (mean sea level) above WGS84 ellipsoid [metres]
    double geoidalSeparation;    
};

//! Vector track and speed over ground data structure.
class VtgData : public GenericData
{
public:
    DataType type() const { return GpVtg; }

    //! Time (according to the computer clock) when data was measured.
    //! Number of seconds
    int timeStampSec;
    //! Time (according to the computer clock) when data was measured.
    //! Number of microseconds
    int timeStampUsec;
    
    //! Heading/track/course with respect to true North [rad]
    double headingTrue; 
    //! Heading/track/course with respect to magnetic North [rad]
    double headingMagnetic; 
    //! Horizontal velocity [metres/second]
    double speed;
};

//! Gps data structure
class RmeData : public GenericData
{
public:
    DataType type() const { return PgRme; }

    //! Time (according to the computer clock) when data was measured.
    //! Number of seconds
    int timeStampSec;
    //! Time (according to the computer clock) when data was measured.
    //! Number of microseconds
    int timeStampUsec;
    
    //! Horizontal position error: one standard deviation [metres)]
    double horizontalPositionError;
    //! Vertical position error: one standard deviation [metres]
    double verticalPositionError;

    //! Estimated position error.
    double estimatedPositionError;
};

/*! 
Garmin GPS driver.

All Garmin receivers understand the latest NMEA standard which is called: 0183 version 2.0. 

This standard dictates a transfer rate of 4800 baud.

This driver can read only the following messages (sentences):
- GPGGA fix data 
- PGRME (estimated error) - not sent if set to 0183 1.5 
- GPVTG vector track and speed over ground 

Processing of individual messages can be disabled in the Config structure.

Note that when fixType contained in the GPGGA is Invalid, all other data in all messages
except the time stamps are meaningless.

Referennces:
- http://en.wikipedia.org/wiki/NMEA
- http://www.gpsinformation.org/dale/interface.htm
*/
class Driver
{

public:

    //! Constructor
    //!
    //! gbxutilacfr::Tracer and gbxutilacfr::Status allow
    //! (human-readable and machine-readable respectively) external
    //! monitorining of the driver's internal state.
    Driver( const Config& config, 
            gbxutilacfr::Tracer& tracer,
            gbxutilacfr::Status& status );

    ~Driver();

/*! 
Blocks till new data is available.

Throws gbxutilacfr::Exception when a problem is encountered.

@verbatim
std::auto_ptr<gbxgarminacfr::GenericData> data;

try {
    data = device->read();
}
catch ( const std::exception& e ) {
    cout <<"Test: Failed to read data: "<<e.what()<<endl;
} 
@endverbatim
*/
    std::auto_ptr<GenericData> read();

private:

    void init();
    void enableDevice();
    void disableDevice();
    
    std::auto_ptr<gbxserialacfr::Serial> serial_;

    Config config_;
    gbxutilacfr::Tracer& tracer_;
    gbxutilacfr::Status& status_;
};

} // namespace

#endif
