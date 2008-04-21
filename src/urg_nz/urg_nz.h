/*
 * Hokuyo URG laser scanner driver class.
 * Originally written by Toby Collett for the Player project.
 */

#include <stdio.h>
#include <inttypes.h>

#include <string>

/** @ingroup gbx_library_urg_nz
@{
*/

namespace urg_nz
{

/** The maximum number of readings the laser is capable of returning. */
const unsigned int MAX_READINGS            = 769;
/** URG_ERR_READPOLL: Error polling the connection to the laser. */
const unsigned int URG_ERR_READPOLL        = 0;
/** URG_ERR_READTIMEOUT: Time out while reading from the laser. */
const unsigned int URG_ERR_READTIMEOUT     = 1;
/** URG_ERR_READ: Read error while reading from the laser. */
const unsigned int URG_ERR_READ            = 2;
/** URG_ERR_PROTOCOL: Error in the SCIP protocol. */
const unsigned int URG_ERR_PROTOCOL        = 3;
/** URG_ERR_CHANGEBAUD: Error while changing baud rate. */
const unsigned int URG_ERR_CHANGEBAUD      = 4;
/** URG_WARN_CHANGEBAUD: Non-fatal error while changing baud rate. */
const unsigned int URG_WARN_CHANGEBAUD     = 5;
/** URG_ERR_BADBAUDRATE: Bad baud rate. */
const unsigned int URG_ERR_BADBAUDRATE     = 6;
/** URG_ERR_OPEN_FAILED: Failed to open a port. */
const unsigned int URG_ERR_OPEN_FAILED     = 7;
/** URG_ERR_CONNECT_FAILED: Failed to connect to the laser. */
const unsigned int URG_ERR_CONNECT_FAILED  = 8;
/** URG_ERR_CLOSE_FAILED: Failed to close a port. */
const unsigned int URG_ERR_CLOSE_FAILED    = 9;
/** URG_ERR_NODESTINATION: No destination buffer provided for range readings. */
const unsigned int URG_ERR_NODESTINATION   = 10;
/** URG_ERR_NOPORT: Port is not open. */
const unsigned int URG_ERR_NOPORT          = 11;
/** URG_ERR_BADFIRMWARE: Bad firmware version. */
const unsigned int URG_ERR_BADFIRMWARE     = 12;
/** URG_ERR_SCIPVERSION: Unknown/unsupported SCIP protocol version. */
const unsigned int URG_ERR_SCIPVERSION     = 13;
/** URG_ERR_LASERERROR: Error reported by laser scanner. */
const unsigned int URG_ERR_LASERERROR      = 14;

/** @brief Range readings from a URG laser scanner. */
typedef struct urg_nz_laser_readings
{
    /** Array of range readings in millimetres. Values less than 20 indicate no return in the scan
    (i.e. there was nothing detected within the laser range). */
    unsigned short Readings[MAX_READINGS];
} urg_nz_laser_readings_t;

/** @brief URG laser scanner configuration information. */
typedef struct urg_nz_laser_config
{
    /** Start angle the laser is capable of scanning from (inclusive), in radians.*/
    float min_angle;
    /** End angle the laser is capable of scanning to (inclusive), in radians.*/
    float max_angle;
    /** Scan resolution (angle between two readings) in radians.  */
    float resolution;
    /** Maximum range in millimetres. */
    float max_range;
} urg_nz_laser_config_t;

/** @brief URG error class. */
class urg_nz_exception
{
    public:
        /** @brief URG error constructor.

        @param code Error code of the error.
        @param desc Description of the error. */
        urg_nz_exception (unsigned int code, std::string desc)
            : error_code (code), error_desc (desc)
        {}

        /** Error code. */
        unsigned int error_code;
        /** Error description. */
        std::string error_desc;
};

/** @brief URG laser scanner class.

Provides an interface for interacting with a Hokuyo URG laser scanner.

To use a serial connection, ensure that you do not also have a USB cable connected, as this will
force the scanner into USB mode, preventing the serial connection from functioning correctly.

All functions may throw instances of @ref urg_nz_exception. */
class urg_laser
{
    public:
        /** @brief Constructor for URG laser scanner class. */
        urg_laser (void);

        /** @brief Destructor for URG laser scanner class. */
        ~urg_laser (void);

        /** @internal Read from the specified file descriptor until a specified number of bytes is received.

        @param fd File descriptor to read from.
        @param buf Pointer to a buffer to store received data in.
        @param len Number of bytes to receive.
        @param timeout Time out for each read performed, in milliseconds.
        @return The number of bytes actually read. */
        int ReadUntil (int fd, unsigned char *buf, int len, int timeout);

        /** @internal Read characters (and throw them away) until the nth occurence of character c.

        @param file File descriptor to read from.
        @param n Number of occurances to read.
        @param c Character to look for.
        @return The number of bytes actually read. */
        int ReadUntil_nthOccurence (int file, int n, char c);

        /** @brief Open a laser scanner on the specified port.

        Supported baud rates for RS232 connections are 19200, 57600 and 115200. Baud rate is
        not applicable to USB connections.

        @param port_name Fully-qualified path to the port the scanner is connected to.
        @param use_serial Use a serial connection. The alternative is termios.
        @param baud Baud rate for serial connections. */
        void Open (const char *port_name, bool use_serial, int baud);

        /** @brief Close the laser scanner connection. */
        void Close (void);

        /** @brief Check if the port to the laser scanner is open.

        @return true if the port is open, false otherwise. */
        bool PortOpen (void);

        /** @brief Change the baud rate of the connection.

        Supported baud rates are 19200, 57600 and 115200. Not applicable to USB connections.

        Note: Not tested with SCIP protocol version 2.

        @param curr_baud The current baud rate of the connection.
        @param new_baud The new baud rate to change the connection to.
        @return 0 on success, non-zero for non-fatal failure. Fatal errors will throw an exception. */
        int ChangeBaud (int curr_baud, int new_baud);

        /** @brief Set the time out for reads performed when talking to the laser.

        A large number of polls are performed while talking to the laser scanner to look for data,
        for example when getting scan data from it. This sets the timeout, in milliseconds, to use
        for these polls. Set it to a negative value for no timeout.

        The default value is -1.

        @param timeout The time out for reads, in milliseconds. */
        void SetTimeOut (int timeout)       { poll_timeout = timeout; }

        /** @brief Retrieve a set of range readings from the scanner. Ranges are returned in millimetres.

        The scan is a series of discrete values. They can be indexed, starting at 0 and going up to
        @ref MAX_READINGS. A subset of these values only can be returned using min_i and max_i. These
        are inclusive, e.g. asking for readings from 5 to 10 will return 6 readings. Typically, you will
        want to at least exclude the readings the scanner can't actually see, as given by
        @ref GetSensorConfig.

        @param readings Pointer to a @ref urg_nz_laser_readings_t structure to store the data in.
        @param min_i The minimum scan index to retrieve. Must be at least 0. Default is 0.
        @param max_i The maximum scan index to retrieve. Must be no greater than @ref MAX_READINGS. Default is @ref MAX_READINGS.
        @return The number of range readings read. */
        unsigned int GetReadings (urg_nz_laser_readings_t *readings, unsigned int min_i = 0, unsigned int max_i = MAX_READINGS);

        /** @brief Get the laser scanner identification information.

        @return The serial number of the laser scanner. */
        int GetIDInfo (void);

        /** @brief Get the laser scanner configuration (resolution, scan angles, etc.)

        @param cfg Pointer to a @ref urg_nz_laser_config_t structure to store the configuration in. */
        void GetSensorConfig (urg_nz_laser_config_t *cfg);

        /** @brief Get the protocol version used by the connected laser scanner.

        Old firmware revisions support protocol SCIP1.0, and a max range of 4 meters.
        Since firmware revision 3.0.00, it's called SCIP2.0, and the max range is 5.6 meters.

        @return The protocol version in use. */
        int GetSCIPVersion (void);

        /** @brief Turns on and off printing of information to the console. Default is off.

        @param verbose_mode Set verbose mode on or off. */
        void SetVerbose (bool verbose_mode)     { verbose = verbose_mode; }
    private:
        /** @internal SCIP protocol version in use. */
        int SCIP_Version;

        /** @internal File structure of the port connected to the laser scanner. */
        FILE *laser_port;

        /** @internal Verbose mode setting. */
        bool verbose;

        /** @internal Poll time out time in milliseconds. */
        int poll_timeout;
};

}; // namespace urg_nz

/** @} */
