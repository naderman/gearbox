/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Matthew Ridley, Ben Upcroft, Michael Moser
 *
 * This distribution is licensed to you under the terms described in
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

/** @ingroup gbx_library_novatel_acfr
@{ */
namespace gbxnovatelacfr{

//! Minimum information to configure the receiver in INS mode
class SimpleConfig{
public:
    //! @param imuType see @ref Config::imuType_ for details
    //! @param imuToGpsOffset see @ref Config::imuToGpsOffset_ for details
    SimpleConfig(std::string serialDevice, int baudRate, std::string imuType, std::vector<double > &imuToGpsOffset):
        serialDevice_(serialDevice),
        baudRate_(baudRate),
        imuType_(imuType),
        imuToGpsOffset_(imuToGpsOffset) {};

    //!@brief Returns true if the configuration is sane.
    //
    //! Checks include:
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

    //!@brief Returns true if the configuration is sane.
    //
    //! Checks include:
    //! - a non-empty device name
    //! - baud rate is supported by device (9600, 19200, 38400, 115200, 230400)
    bool isValid() const;
    std::string toString();

    std::string serialDevice_;
    int baudRate_;
};

//! @brief All the information needed to configure the driver.
//
//! The device itself has even more options, consult your manual.
//! If all these possibilities don't seem to be sufficient, consult your friendly developer for extension (better yet, send a patch)
//! The easiest way to get a valid (and useful) Config, is to initialise it with a SimpleConfig (for INS operation) or GpsOnlyConfig (for GPS operation)
class Config{
public:
    //! @brief Shortcut to set up a Configuration for INS operation
    //
    //! yields a valid config, with reasonable defaults (RawImu 100Hz, InsPva 50Hz, BestGpsPos/Vel 5Hz)
    Config(const SimpleConfig &simpleCfg);
    //! @brief Shortcut to set up a Configuration for GPS operation
    //
    //! yields a valid config, for gps only operation (BestGpsPos/Vel 20Hz)
    Config(const GpsOnlyConfig &gpsOnlyCfg); 
    //! disables everything, so you can (and must) set just the options you need.
    Config();

    //! @brief Returns true if the configuration is sane.
    //
    //! Checks include:
    //! - a non-empty device name
    //! - baud rate is supported by device (9600, 19200, 38400, 57600, 115200, 230400)
    //! - imuType refers to a known type
    //! - offset has size 3
    //! - message rates are consistent and don't exceed serial-data-rate
    //! - not const, since it has limited self-fixing capabilities (for incorrect message-rates)
    bool isValid();
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
    std::string imuType_; //!< as expected by the "SETIMUTYPE" command (SPAN Technology for OEMV User Manual Rev 3, Table 15, page 64)
    //!@}

    //!@name Data settings
    //! First we disable all output in the ctor. Then all the messages set to true here are  enabled.
    //
    //!@{
    bool enableInsPva_;
    bool enableGpsPos_;
    bool enableGpsVel_;
    bool enableRawImu_;
    bool ignoreUnknownMessages_; //!< normally we throw an exception, set this to "true" if you want to enable some other message permanently.

    //!@}

    //!@name Data rate settings
    //!Time between messages in seconds (i.e. 0.01 == 100Hz). RawImu can only be reported at
    //!the "natural" rate of the IMU (100Hz or 200Hz, depending on model).
    //!We check in isValid() if any of these don't make sense
    //
    //!@{
    double dtInsPva_; //!< 100Hz max, if RawImu is enabled 50Hz max
    double dtGpsPos_; //!< 20Hz max, 5Hz max if RawImu or InsPva is enabled
    double dtGpsVel_; //!< 20Hz max, 5Hz max if RawImu or InsPva is enabled
    bool fixInvalidRateSettings_; //!< Don't bitch about wrong rates, change them to something sensible

    //!@}

    //!@name INS settings
    //
    //!@{
    std::vector<double > imuToGpsOffset_;            //!< vector (xyz [m]) from IMU center to Antenna Phase Center, in IMU coordinates, vital for INS performance, make sure you get this right!
    std::vector<double > imuToGpsOffsetUncertainty_; //!< optional (size 3 or 0) xyz; !it is unclear to me whether these are factors, or absolute values (in [m])!
    bool enableInsOffset_;
    std::vector<double > insOffset_;                 //!< report INS position/velocity offset (xyz [m] in IMU coordinates) from the IMU center; useful e.g. to get data w.r. to robot's center of rotation
    bool enableInsPhaseUpdate_;                      //!< tightly coupled (phase based vs position based) filter; Chance of better performance in adverse conditions

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
    //! These guys enable the Span system to do an alignment while moving, they also allow you to mount the IMU in weird ways (e.g. upside down).
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
enum StatusMessageType {
    NoMsg,       //!< Nothing new, no message
    Initialising,//!< Nothing wrong, just not quite ready
    Ok,          //!< All good, but something to say
    Warning,     //!< Problem, likely to go away
    Fault        //!< Problem, probably fatal
};

//! Novatel's different solution status types.
//
//! Explanations from the manual.
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
    ReservedGpsSolutionStatusType,
    UnknownGpsSolutionStatusType
};

//! Novatel's different fix types.
//
//! Sadly mixed for position/velocity with some INS gear thrown in; explanations from the manual.
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
    ReservedGpsPosVelType,
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
class GenericData {
    public:
        virtual ~GenericData(){};
        virtual DataType type() const=0;
        virtual std::string toString()=0;
    private:
};

//! INS position/velocity/attitude information
class InsPvaData : public GenericData {
    public:
        DataType type() const {
            return InsPva;
        }
        std::string toString();
        int      gpsWeekNr;         //!< number of full weeks since midnight 05/Jan/1980 (UTC)
        double   secIntoWeek;       //!< yields GPS-time (together with @ref gpsWeekNr); continous (contrary to UTC which uses leapseconds)
        double   latitude;          //!< [deg] north positive WGS84
        double   longitude;         //!< [deg] east positive WGS84
        double   height;            //!< [m] above ellipsoid WGS84 (heigth_ellipsoid - undulation == height_geoid (aka AMSL)

        //!@name Velocity vector
        //!Relativ to true North/East and geoid vertical (I think)
        //
        //@{
        double   northVelocity;     //!< [m/s] south is negative
        double   eastVelocity;      //!< [m/s] west is negative
        double   upVelocity;        //!< [m/s] down is negative

        //@}

        //!@name Orientation
        //!The default IMU axis definitions are: Y - forward Z - up X - right hand side
        //
        //@{
        double   roll;              //!< [degree] right handed rotation from local level around y-axes
        double   pitch;             //!< [degree] right handed rotation from local level around x-axes
        double   azimuth;           //!< [degree] left handed around z-axes rotation from (true?) north clockwise

        //@}

        StatusMessageType statusMessageType;
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
        std::string toString();
        int gpsWeekNr;                          //!< number of full weeks since midnight 05/Jan/1980 (UTC)
        unsigned int msIntoWeek;                //!< yields GPS-time (together with @ref gpsWeekNr); continous (contrary to UTC which uses leapseconds)
        GpsSolutionStatusType  solutionStatus;  //
        GpsPosVelType          positionType;    //
        double       latitude;                  //!< [deg] north positive
        double       longitude;                 //!< [deg] east positive
        double       heightAMSL;                //!< [m] AMSL == above mean sea level (geoid)
        float        undulation;                //!< [m] aka geoidal seperation: undulation == heigth_ellipsoid - height_geoid/AMSL
        unsigned int datumId;                   //
        float        sigmaLatitude;             //!< [m] 1 standard deviation error estimate
        float        sigmaLongitude;            //!< [m] 1 standard deviation error estimate
        float        sigmaHeight;               //!< [m] 1 standard deviation error estimate
        char         baseStationId[4];          //
        float        diffAge;                   //!< [s] how old the correction info from the basestation is
        float        solutionAge;               //!< [s] 
        int          numObservations;           //!< number of observations tracked (?) L1 code/carrier/doppler + L2 code/carrier/doppler?
        int          numL1Ranges;               //!< number of L1 ranges used in computation (?)
        int          numL1RangesRTK;            //!< number of L1 ranges above the RTK mask angle (??) number of L1 carrier ranges used?
        int          numL2RangesRTK;            //!< number of L2 ranges above the RTK mask angle (??) number of L2 carrier ranges used?

        StatusMessageType statusMessageType;
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
        std::string toString();
        int          gpsWeekNr;                 //!< number of full weeks since midnight 05/Jan/1980 (UTC)
        unsigned int msIntoWeek;                //!< yields GPS-time (together with @ref gpsWeekNr); continous (contrary to UTC which uses leapseconds)
        GpsSolutionStatusType  solutionStatus;  //
        GpsPosVelType          velocityType;    //
        float        latency;                   //!< [s] gps speed can be calculated from instantanious or integrated doppler. The latter refers to the average speed over the last interval -> is delayed by half an interval
        float        diffAge;                   //!< [s]
        double       horizontalSpeed;           //!< [m/s]
        double       trackOverGround;           //!< [deg] "heading" of the speed vector w. respect to true North
        double       verticalSpeed;             //!< [m/s]

        StatusMessageType statusMessageType;
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
        std::string toString();
        int    gpsWeekNr;   //!< number of full weeks since midnight 05/Jan/1980 (UTC)
        double secIntoWeek; //!< yields GPS-time (together with @ref gpsWeekNr); continous (contrary to UTC which uses leapseconds)
        //!@name Change in speed
        //!Divide by dt to get accelerations.
        //!The default IMU axis definitions are: Y - forward, Z - up, X - right hand side
        //
        //@{
        double zDeltaV;   //!< [m/s] up positive
        double yDeltaV;   //!< [m/s] forward positive
        double xDeltaV;   //!< [m/s] right positive

        //@}

        //!@name Change in orientation
        //!Divide by dt to get turn-rates.
        //!The default IMU axis definitions are: Y - forward, Z - up, X - right hand side
        //
        //@{
        double zDeltaAng; //!< [rad] right handed around z
        double yDeltaAng; //!< [rad] right handed around y
        double xDeltaAng; //!< [rad] right handed around x

        //@}

        StatusMessageType statusMessageType;
        std::string statusMessage;

        int timeStampSec;  //!< in Computer time, beginning of message at serial port
        int timeStampUSec; //!< in Computer time, beginning of message at serial port
};

//! The actual Driver
//! @brief the idea is to create one of these guys (with a valid Config), and then treat it as a data-source, i.e. call read() on it in some kind of loop
class Driver {
public:

    //! @name Ctor:
    //! @brief Tries to establish serial communication to the GPS receiver, then configures it
    //
    //!@{

    //! @brief dumps tracing messages to the console
    //
    //! implements tracing internally as a gbxutilacfr::TrivialTracer
    Driver( const Config &cfg);
    //!@brief full control over tracing (e.g. to syslog)
    //!@param tracer you need to provide this guy (via new), do NOT delete it, it will be deleted when your Driver object goes out of scope
    Driver( const Config &cfg, gbxutilacfr::Tracer* tracer);
    //!@}
    ~Driver();

    /*! @brief Blocking read, returns one message

        Throws gbxutilacfr::Exception when a problem is encountered (derives from std::exception).
        Throws gbxutilacfr::HardwareException when a (fatal) problem with the hardware is encountered
        */
    std::auto_ptr<GenericData> read();

private:

    // does the leg-work for the constructor (via the following guys)
    void configure();
    // establish a serial connection to the receiver
    void connectToHardware();
    // set parameters related to the IMU
    void configureImu();
    // set parameters related to the INS
    void configureIns();
    // set parameters related to GPS
    void configureGps();
    // turn on data messages we are interested in
    void requestData();

    std::auto_ptr<gbxnovatelutilacfr::ImuDecoder> imuDecoder_;

    std::auto_ptr<gbxserialacfr::Serial> serial_;
    int baud_;

    Config config_;
    std::auto_ptr<gbxutilacfr::Tracer > tracer_;
};


} // namespace
/** @} */
#endif
