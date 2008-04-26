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
#include <gbxgarminacfr/gbxgpsutilacfr/nmea.h>
#include <memory>

namespace gbxgarminacfr {

//! Configuration structure
class Config
{   
public:
    Config() {};
    bool isValid() const;
    std::string toString() const;

    //! Serial device. e.g. "/dev/ttyS0"
    std::string device;
};


//! Gps position types.
//! Using Novatel codes here is probably not the best thing. With more
//! thought it's probably possible to categorize these position types
//! into more generic categories. For now, non-Novatel receivers should
//! use the generic types listed first.
enum PositionType {
    //! Invalid or not available
    GpsPositionTypeNotAvailable,
    //! Autonomous position
    //! (This is the normal case for non-differential GPS)
    GpsPositionTypeAutonomous,
    //! Differentially corrected
    GpsPositionTypeDifferential,
    NovatelNone,
    NovatelFixedPos,
    NovatelFixedHeigth,
    NovatelFloatConv,
    NovatelWideLane,
    NovatelNarrowLane,
    NovatelDopplerVelocity,
    NovatelSingle,
    NovatelPsrDiff,
    NovatelWAAS,
    NovatelPropagated,
    NovatelOmnistar,
    NovatelL1Float,
    NovatelIonFreeFloat,
    NovatelNarrowFloat,
    NovatelL1Int,
    NovatelWideInt,
    NovatelNarrowInt,
    NovatelRTKDirectINS,
    NovatelINS,
    NovatelINSPSRSP,
    NovatelINSPSRFLOAT,
    NovatelINSRTKFLOAT,
    NovatelINSRTKFIXED,
    NovatelOmnistarHP,
    NovatelUnknown
};

//! Gps data structure
struct Data
{
    //! Time (according to the computer clock) when data was measured.
    //! Number of seconds
    int timeStampSec;
    //! Time (according to the computer clock) when data was measured.
    //! Number of microseconds
    int timeStampUsec;
    //! UTC time (according to GPS device), reference is Greenwich.
    //! Hour [0..23]
    int utcTimeHrs;
    //! UTC time (according to GPS device), reference is Greenwich.
    //! Minutes [0..59]
    int utcTimeMin;
    //! UTC time (according to GPS device), reference is Greenwich.
    //! Seconds [0.0..59.9999(9)]
    double utcTimeSec;

    //! Latitude (degrees)
    double latitude;
    //! Longitude (degrees)
    double longitude;
    //! Altitude (metres above ellipsoid)
    double altitude;
    
    //! Horizontal position error: one standard deviation (metres)
    double horizontalPositionError;
    //! Vertical position error: one standard deviation (metres)
    double verticalPositionError;
    
    //! Heading/track/course with respect to true north (rad)
    double heading; 
    //! Horizontal velocity (metres/second)
    double speed;
    //! Vertical velocity (metres/second)
    double climbRate;
    
    //! Number of satellites
    int satellites;
    int observationCountOnL1;
    int observationCountOnL2;
    //! Position type (see above)
    PositionType positionType;
    //! Geoidal Separation (metres)
    double geoidalSeparation;    
};


//! Garmin driver
class Driver
{

public:

    //! Constructor
    //!
    //! gbxutilacfr::Tracer and gbxutilacfr::Status allow
    //! (human-readable and machine-readable respectively) external
    //! monitorining of the driver's internal state.
    Driver( const Config &config, 
            gbxsickacfr::gbxutilacfr::Tracer &tracer,
            gbxsickacfr::gbxutilacfr::Status &status );

    ~Driver();


    //! Blocks till new data is available
    void read( Data &data );

private:

    void init();
    void addDataToFrame();
    void enableDevice();
    void disableDevice();
    int  resetDevice();
    void extractGGAData();
    void extractVTGData();
    void extractRMEData();
    void clearFrame(){haveGGA_ = false; haveVTG_ = false; haveRME_ =false;};
    bool haveCompleteFrame(){return (haveGGA_ & haveVTG_ & haveRME_);};
    
    void readFrame( Data &data);
    
    std::auto_ptr<gbxserialacfr::Serial> serial_;

    Data gpsData_;
    gbxgpsutilacfr::NmeaMessage nmeaMessage_;
    int timeOfReadSec_;
    int timeOfReadUsec_;
    
    //*** NOTE:- if we change the number of messages in the frame need to change
    //The N_MSGS_IN_FRAME in the readFrame fn...
    bool haveGGA_;
    bool haveVTG_;
    bool haveRME_;

    Config config_;
    gbxsickacfr::gbxutilacfr::Tracer& tracer_;
    gbxsickacfr::gbxutilacfr::Status& status_;
};

} // namespace

#endif
