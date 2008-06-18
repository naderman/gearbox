/*
 * Orca-Robotics Project: Components for robotics 
 *               http://orca-robotics.sf.net/
 * Copyright (c) 2004-2008 Matthew Ridley, Ben Upcroft, Michael Moser
 *
 * This copy of Orca is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBXNOVATELACFR_DRIVER_H
#define GBXNOVATELACFR_DRIVER_H

#include <cstdlib>
#include <string>
#include <memory>
#include <vector>

// forward declarations
// users don't need to know about serial devices or Tracers or imu decoders
namespace gbxserialacfr{
    class Serial;
}
namespace gbxutilacfr{
    class Tracer;
}
namespace gbxnovatelutilacfr{
    class ImuDecoder;
}

namespace gbxnovatelacfr{

//! Minimum information to configure the receiver in INS mode
class SimpleConfig{
public:
    //! @parameter imuToGpsOffset: vector (xyz [m]) from IMU center to Antenna Phase Center
    //! in IMU coordinates, vital for INS performance, make sure you get this right!
    //! @parameter imuType: as expected by the "SETIMUTYPE" command (SPAN Technology for OEMV User Manual Rev 3, Table 15, page 64)
    SimpleConfig(std::string serialDevice, int baudRate, std::string imuType, std::vector<double > &imuToGpsOffset):
        serialDevice_(serialDevice),
        baudRate_(baudRate),
        imuType_(imuType),
        imuToGpsOffset_(imuToGpsOffset) {};

    //! Returns true if the configuration is sane. Checks include:
    //! - a non-empty device name
    //! - baud rate is supported by device (9600, 19200, 38400, 115200, 230400)
    //! - imuType refers to a known type
    //! - offset has size 3
    bool isValid() const;
    //! Dumps the config in human readable form
    std::string toString();

    std::string serialDevice_;
    int baudRate_;
    std::string imuType_;
    std::vector<double > imuToGpsOffset_;
};

//! Minimum information needed to configure the receiver in GPS only mode
class GpsOnlyConfig{
public:
    GpsOnlyConfig(std::string serialDevice, int baudRate):
        serialDevice_(serialDevice),
        baudRate_(baudRate) {};

    //! Returns true if the configuration is sane. Checks include:
    //! - a non-empty device name
    //! - baud rate is supported by device (9600, 19200, 38400, 115200, 230400)
    bool isValid() const;
    std::string toString();

    std::string serialDevice_;
    int baudRate_;
};

//! All the information needed to configure the driver. The device itself has even more options, consult your manual.
//! If all these possibilities don't seem to be sufficient, consult your friendly developer for extension (better yet, send a patch)
class Config{
public:
    Config(const SimpleConfig &simpleCfg);   //!< yields a valid config, with reasonable defaults
    Config(const GpsOnlyConfig &gpsOnlyCfg); //!< yields a valid config, for gps only operation
    Config();                                //!< disables everything, so you can set just the options you need

    //! Returns true if the configuration is sane. Checks include:
    //! - a non-empty device name
    //! - baud rate is supported by device (9600, 19200, 38400, 115200, 230400)
    //! - imuType refers to a known type
    //! - offset has size 3
    //! - message rates are consistent and don't exceed serial-data-rate
    bool isValid() const;
    //! Dumps the config in human readable form
    std::string toString();

    //!@name Serial settings
    //
    //!@{
    std::string serialDevice_;
    int baudRate_;
    //!@}

    //!@name IMU settings
    //
    //!@{
    bool enableImu_;
    std::string imuType_;
    //!@}

    //!@name Data settings
    //
    //!we disable all output in the ctor, and then enable the messages set to true here.
    //!@{
    bool enableInsPva_;
    bool enableGpsPos_;
    bool enableGpsVel_;
    bool enableRawImu_;
    bool ignoreUnknownMessages_; //!< normally we throw an exception, set this to "true" if you want to enable some other message permanently.
    //!@}

    //!@name Data rate settings
    //
    //!Time between messages in seconds (i.e. 0.01 == 100Hz). RawImu can only be reported at
    //!the "natural" rate of the IMU (100Hz or 200Hz, depending on model).
    //!We check in isValid() if any of these don't make sense
    //!@{
    double dtInsPva_; //!< 100Hz max, if RawImu is enabled 50Hz max
    double dtGpsPos_; //!< 20Hz max, 5Hz max if RawImu or InsPva is enabled
    double dtGpsVel_; //!< 20Hz max, 5Hz max if RawImu or InsPva is enabled
    bool fixInvalidRateSettings_; //!< don't bitch about wrong rates, but change them to something sensible
    //!@}

    //!@name INS settings
    //
    //!@{
    std::vector<double > imuToGpsOffset_;
    std::vector<double > imuToGpsOffsetUncertainty_; //!< optional (size 3 or 0)
    bool enableInsOffset_;
    std::vector<double > insOffset_;         //!< report INS position/velocity offset (xyz [m] in IMU coordinates) from the IMU center; useful e.g. to get data w.r. to robot's center of rotation
    bool enableInsPhaseUpdate_;         //!< tightly coupled (phase based vs position based) filter; Chance of better performance in adverse conditions
    //!@}

    //!@name GPS settings
    //
    //!@{
    bool enableCDGPS_; //!< code-differential corrections over satellite (North America/Canada)
    bool enableSBAS_;  //!< code-differential corrections over satellite on GPS frequencies (WAAS/EGNOS)
    bool enableRTK_;   //!< carrier-differential corrections (you need to set up your own base-station and wireless link), assumes RTCA corrections on COM2, 9600bps, 8N1 (hardcoded)
    bool enableUseOfOmniStarCarrier_; //!< carrier-differential corrections OMNIStarXP/HP (you need to get a subscription with them)
    //!@}

    //!@name INS settings for fast (dynamic) alignment
    //! !I'd strongly recommend that you read the manual _very_ closely!
    //! These guys enbale the Span system to do an alignment while moving, they also allow you to mount the IMU in weird ways (e.g. upside down).
    //! It's worth to accept a fair amount of pain to mount the IMU in the recommended way. Otherwise you'll probably need a good amount of
    //! experimentation/calibration to get a parameter-set that works.
    //
    //!@{
    bool enableSetImuOrientation_;
    int setImuOrientation_;
    bool enableVehicleBodyRotation_;
    std::vector<double > vehicleBodyRotation_;
    std::vector<double > vehicleBodyRotationUncertainty_; //!< optional (size 3 or 0)
    //!@}
private:
};

//! possible Status Messages GenericData can contain
enum StatusMessagetype {
    //! Nothing new, no message
    NoMsg,
    //! All good, but something to say
    Ok,
    //! Problem, likely to go away
    Warning,
    //! Problem, probably fatal
    Fault
};

//! Novatel's different solution status types
enum GpsSolutionStatusType{
    SolComputed,       //!< Solution computed
    InsufficientObs,   //!< Insufficient observations
    NoConvergence,     //!< No convergence
    Singularity,       //!< Singularity at parameters matrix
    CovTrace,          //!< Covariance trace exceeds maximum (trace > 1000 m)
    TestDist,          //!< Test distance exceeded (maximum of 3 rejections if distance > 10 km)
    ColdStart,         //!< Not yet converged from cold start
    VHLimit,           //!< Height or velocity limits exceeded (in accordance with COCOM export licensing restrictions)
    Variance,          //!< Variance exceeds limits
    Residuals,         //!< Residuals are too large
    DeltaPos,          //!< Delta position is too large
    NegativeVar,       //!< Negative variance
    IntegrityWarning,  //!< Large residuals make position unreliable
    InsInactive,       //!< INS has not started yet
    InsAligning,       //!< INS doing its coarse alignment
    InsBad,            //!< INS position is bad
    ImuUnplugged,      //!< No IMU detected
    Pending,           //!< When a FIX POSITION command is entered, the receiver computes its own position and determines if the fixed position is valid
    InvalidFix,        //!< The fixed position, entered using the FIX POSITION command, is not valid
    UnknownGpsSolutionStatusType
};

//! Novatel's different fix types; sadly mixed for position/velocity with some INS gear thrown in
enum GpsPosVelType{
    None,             //!< No solution
    FixedPos,         //!< Position has been fixed by the FIX POSITION command or by position averaging
    FixedHeight,      //!< Position has been fixed by the FIX HEIGHT, or FIX AUTO, command or by position averaging
    FloatConv,        //!< Solution from floating point carrier phase ambiguities
    WideLane,         //!< Solution from wide-lane ambiguities
    NarrowLane,       //!< Solution from narrow-lane ambiguities
    DopplerVelocity,  //!< Velocity computed using instantaneous Doppler
    Single,           //!< Single point position
    PsrDiff,          //!< Pseudorange differential solution
    Waas,             //!< Solution calculated using corrections from an SBAS
    Propagated,       //!< Propagated by a Kalman filter without new observations
    Omnistar,         //!< OmniSTAR VBS position (L1 sub-meter) a
    L1Float,          //!< Floating L1 ambiguity solution
    IonoFreeFloat,    //!< Floating ionospheric-free ambiguity solution
    NarrowFloat,      //!< Floating narrow-lane ambiguity solution
    L1Int,            //!< Integer L1 ambiguity solution
    WideInt,          //!< Integer wide-lane ambiguity solution
    NarrowInt,        //!< Integer narrow-lane ambiguity solution
    RtkDirectIns,     //!< RTK status where the RTK filter is directly initialized from the INS filter. b
    Ins,              //!< INS calculated position corrected for the antenna b
    InsPsrSp,         //!< INS pseudorange single point solution - no DGPS corrections b
    InsPsrDiff,       //!< INS pseudorange differential solution b
    InsRtkFloat,      //!< INS RTK floating point ambiguities solution b
    InsRtkFixed,      //!< INS RTK fixed ambiguities solution b
    OmniStarHp,       //!< OmniSTAR high precision a
    OmniStarXp,       //!< OmniSTAR extra precision a
    CdGps,            //!< Position solution using CDGPS corrections
    UnknownGpsPosVelType
};


//! possible types GenericData can contain
enum DataType {
    //! GenericData is really InsPvaData
    InsPva,
    //! GenericData is really BestGpsPosData
    BestGpsPos,
    //! GenericData is really BestGpsVelData
    BestGpsVel,
    //! GenericData is really RawImuData
    RawImu
};

//! Generic (base) type returned by a read
class GenericData
{
    public:
        virtual ~GenericData(){};
        virtual DataType type() const=0;
        virtual std::string toString() const=0;
    private:
};

//! INS position/velocity/attitude information
class InsPvaData : public GenericData {
    public:
        DataType type() const {
            return InsPva;
        }
        std::string toString() const{
            return "implement me!";
        }
        int      gpsWeekNr;         //
        double   secIntoWeek;       //
        double   latitude;          //[deg] north positive WGS84
        double   longitude;         //[deg] east positive WGS84
        double   height;            //[m] above ellipsoid WGS84 (heigth_ellipsoid - undulation == height_geoid/AMSL)
        double   northVelocity;     //[m/s] south is negative; true north?
        double   eastVelocity;      //[m/s] west is negative; true east?
        double   upVelocity;        //[m/s] down is negative; geoid/ellipsoid vertical?
        //The default IMU axis definitions are:
        //  Y - forward
        //  Z - up
        //  X - right hand side
        double   roll;              //[degree] right handed rotation from local level around y-axes
        double   pitch;             //[degree] right handed rotation from local level around x-axes
        double   azimuth;           //[degree] left handed around z-axes rotation from (true?) north clockwise

        StatusMessagetype statusMessageType;
        std::string statusMessage;

        int timeStampSec;  //!< in Computer time, beginning of message at serial port
        int timeStampUSec; //!< in Computer time, beginning of message at serial port
};

//! Gps position information
class BestGpsPosData : public GenericData {
    public:
        DataType type() const {
            return BestGpsPos;
        }
        std::string toString() const{
            return "implement me!";
        }
        int gpsWeekNr;                          //
        unsigned int msIntoWeek;                //milliseconds from beginning of week
        GpsSolutionStatusType  solutionStatus;  //
        GpsPosVelType          positionType;    //
        double       latitude;                  //[deg] north positive
        double       longitude;                 //[deg] east positive
        double       heightAMSL;                //[m] AMSL == above mean sea level (geoid)
        float        undulation;                //[m] aka geoidal seperation: undulation == heigth_ellipsoid - height_geoid/AMSL
        unsigned int datumId;                   //
        float        sigmaLatitude;             //[m? deg?] 1 standard deviation error estimate
        float        sigmaLongitude;            //[m? deg?] 1 standard deviation error estimate
        float        sigmaHeight;               //[m? deg?] 1 standard deviation error estimate
        char         baseStationId[4];          //
        float        diffAge;                   //[s]
        float        solutionAge;               //[s]
        int          numObservations;           //number of observations tracked (?) L1 code/carrier/doppler + L2 code/carrier/doppler?
        int          numL1Ranges;               //number of L1 ranges used in computation (?)
        int          numL1RangesRTK;            //number of L1 ranges above the RTK mask angle (??) number of L1 carrier ranges used?
        int          numL2RangesRTK;            //number of L2 ranges above the RTK mask angle (??) number of L2 carrier ranges used?

        StatusMessagetype statusMessageType;
        std::string statusMessage;

        int timeStampSec;  //!< in Computer time, beginning of message at serial port
        int timeStampUSec; //!< in Computer time, beginning of message at serial port
};

//! Gps velocity information
class BestGpsVelData : public GenericData {
    public:
        DataType type() const {
            return BestGpsVel;
        }
        std::string toString() const{
            return "implement me!";
        }
        int          gpsWeekNr;                 //
        unsigned int msIntoWeek;                //milliseconds from beginning of week
        GpsSolutionStatusType  solutionStatus;  //
        GpsPosVelType          positionType;    //
        float        latency;                   //[s]
        float        diffAge;                   //[s]
        double       horizontalSpeed;           //[m/s]
        double       trackOverGround;           //[deg] w respect to true North
        double       verticalSpeed;             //[m/s]

        StatusMessagetype statusMessageType;
        std::string statusMessage;

        int timeStampSec;  //!< in Computer time, beginning of message at serial port
        int timeStampUSec; //!< in Computer time, beginning of message at serial port
};

//! Raw IMU information
class RawImuData : public GenericData {
    public:
        DataType type() const {
            return RawImu;
        }
        std::string toString() const{
            return "implement me!";
        }
        int    gpsWeekNr;
        double secIntoWeek;
        double zDeltaV;   //!< [m/s] change in speed, up positive
        double yDeltaV;   //!< [m/s] change in speed, forward positive
        double xDeltaV;   //!< [m/s] change in speed, right positive
        double zDeltaAng; //!< [rad] change in angle, right handed around z
        double yDeltaAng; //!< [rad] change in angle, right handed around y
        double xDeltaAng; //!< [rad] change in angle, right handed around x

        StatusMessagetype statusMessageType;
        std::string statusMessage;

        int timeStampSec;  //!< in Computer time, beginning of message at serial port
        int timeStampUSec; //!< in Computer time, beginning of message at serial port
};

class Driver {
public:

    Driver( const Config &cfg, gbxutilacfr::Tracer& tracer);
    ~Driver();

    //! Blocking read, returns one message
    //! Throws gbxutilacfr::Exception when a problem is encountered.
    //!
    //
    //! @verbatim
    //! std::auto_ptr<gbxnovatelacfr::GenericData> data;
    //!
    //! try {
    //!     data = device->read();
    //! }
    //! catch ( const std::exception& e ) {
    //!     cout <<"Test: Failed to read data: "<<e.what()<<endl;
    //! }
    //! if(InsData == data.type()){
    //!     InsData *insData = dynamic_cast<InsData *>(data.get());
    //!     assert(insData);
    //!     //process insData
    //! }
    //! @endverbatim
    //
    std::auto_ptr<GenericData> read();

private:

    //! establish a serial connection to the receiver
    int connectToHardware();
    //! transfer configuration parameters related to the IMU
    void configureImu();
    //! transfer configuration parameters related to the INS
    void configureIns();
    //! transfer configuration parameters related to GPS
    void configureGps();
    //! turn on data messages we are interested in
    void requestData();

    std::auto_ptr<gbxnovatelutilacfr::ImuDecoder> imuDecoder_;

    std::auto_ptr<gbxserialacfr::Serial> serial_;
    int baud_;

    Config config_;
    gbxutilacfr::Tracer& tracer_;
};


} // namespace
#endif
