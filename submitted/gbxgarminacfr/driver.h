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
#include <gbxsickacfr/gbxutilacfr/tracer.h>
#include <gbxsickacfr/gbxutilacfr/status.h>
#include <memory>

namespace gbxgarminacfr {

//! Configuration structure
class Config
{   
public:
    Config() :
        readGga(true),
        readVtg(true),
        readRme(true) {};

    //! Returns true if the configuration is sane. Checks include:
    //! - a non-empty device name
    //! - at least one read-sentence flag set to TRUE
    bool isValid() const;

    //! Returns human-readable configuration description.
    std::string toString() const;

    //! Serial device. e.g. "/dev/ttyS0"
    std::string device;

    //! Read PGGGA sentence
    bool readGga;
    //! Read PGVTG sentence
    bool readVtg;
    //! Read GPRME sentence
    bool readRme;
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

//! Possible Status Messages GenericData can contain
enum StatusMessageType 
{
    //! Nothing new, no message
    NoMsg,
    //! All good, but something to say
    Ok,
    //! Problem, likely to go away
    Warning,
    //! Problem, probably fatal
    Fault
};

//! Generic data type returned by a read
class GenericData
{
public:
    virtual ~GenericData(){};
    //! Returns data type.
    virtual DataType type() const=0;
    //! Status message type.
    StatusMessageType statusMessageType;
    //! Status message
    std::string statusMessage;

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

//! Fix data structure
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
    
    //! Fix type.
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
    //! Vertical velocity [metres/second]
    double climbRate; 
};

    enum VTGTokens{MsgType=0,HError,M1,VError,M2,EPE,M3};

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
};

/*! 

Garmin GPS driver


Referennces:
- http://en.wikipedia.org/wiki/NMEA
- http://www.gpsinformation.org/dale/interface.htm

All Garmin receivers understand the latest standard which is called: 0183 version 2.0. 

This standard dictates a transfer rate of 4800 baud.

Out of all the messages below, this driver can read only the following messages (sentences):
- GPGGA
- GPVTG
- PGRME

The sentences sent by Garmin receivers include:
NMEA 2.0 
- GPBOD bearing, origin to destination - earlier G-12's do not transmit this 
- GPGGA fix data 
- GPGLL Lat/Lon data - earlier G-12's do not transmit this 
- GPGSA overall satellite reception data 
- GPGSV detailed satellite data 
- GPRMB minimum recommended data when following a route 
- GPRMC minimum recommended data 
- GPRTE route data 
- GPWPL waypoint data (this is bidirectional)

NMEA 1.5 - some units do not support version 1.5 
- GPBOD bearing origin to destination - earlier G-12's do not send this 
- GPBWC bearing to waypoint using great circle route. 
- GPGLL lat/lon - earlier G-12's do not send this 
- GPRMC minimum recommend data 
- GPRMB minimum recommended data when following a route 
- GPVTG vector track and speed over ground 
- GPWPL waypoint data (only when active goto) 
- GPXTE cross track error

In addition Garmin receivers send the following Proprietary Sentences: 
- PGRME (estimated error) - not sent if set to 0183 1.5 
- PGRMM (map datum) 
- PGRMZ (altitude) 
- PSLIB (beacon receiver control)

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
            gbxsickacfr::gbxutilacfr::Tracer& tracer,
            gbxsickacfr::gbxutilacfr::Status& status );

    ~Driver();

    //! Blocks till new data is available
    std::auto_ptr<GenericData> read();

private:

    void init();
    void enableDevice();
    void disableDevice();
    
    std::auto_ptr<gbxserialacfr::Serial> serial_;

    Config config_;
    gbxsickacfr::gbxutilacfr::Tracer& tracer_;
    gbxsickacfr::gbxutilacfr::Status& status_;
};

} // namespace

#endif
