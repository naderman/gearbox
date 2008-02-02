/*
 * Hokuyo URG laser scanner driver class.
 * Originally written by Toby Collett for the Player project.
 */

#include <stdio.h>
#include <inttypes.h>

/** @ingroup gbx_library_urglaser
@{
*/

#define MAX_READINGS 769

namespace urglaser
{

/** @brief Range readings from a URG laser scanner. */
typedef struct urg_laser_readings
{
    unsigned short Readings[MAX_READINGS];
} urg_laser_readings_t;

/** @brief URG laser scanner configuration information. */
typedef struct urg_laser_config
{
    /** Start and end angles for the laser scan [rad].*/
    float min_angle;
    /** Start and end angles for the laser scan [rad].*/
    float max_angle;
    /** Scan resolution [rad].  */
    float resolution;
    /** Maximum range [mm] */
    float max_range;
} urg_laser_config_t;

/** @brief URG laser scanner class.

Provides an interface for interacting with a Hokuyo URG laser scanner. */
class urg_laser
{
    public:
        /** @brief Constructor for URG laser scanner class. */
        urg_laser (void);

        /** @brief Destructor for URG laser scanner class. */
        ~urg_laser (void);

        /** @brief Open a laser scanner on the specified port.

        Supported baud rates are 19200, 57600 and 115200.

        @param PortName Fully-qualified path to the port the scanner is connected to.
        @param use_serial Use a serial connection. The alternative is termios.
        @param baud Baud rate for serial connections. */
        int Open (const char *port_name, bool use_serial, int baud);

        /** @brief Close the laser scanner connection. */
        int Close (void);

        /** @brief Change the baud rate of the connection.

        Supported baud rates are 19200, 57600 and 115200.

        Not applicable to termios connections?

        @param curr_baud The current baud rate of the connection.
        @param new_baud The new baud rate to change the connection to.
        @param timeout Time out for changing the speed, in milliseconds. */
        int ChangeBaud (int curr_baud, int new_baud, int timeout);

        /** @internal Read from the specified file descriptor until a specified number of bytes is received.

        @param fd File descriptor to read from.
        @param buf Pointer to a buffer to store received data in.
        @param len Number of bytes to receive.
        @param timeout Time out for each read performed, in milliseconds. */
        int ReadUntil (int fd, unsigned char *buf, int len, int timeout);

        /** @internal Read characters (and throw them away) until the nth occurence of character c.

        @param file File descriptor to read from.
        @param n Number of occurances to read.
        @param c Character to look for. */
        int ReadUntil_nthOccurence (int file, int n, char c);

        /** @brief Check if the port to the laser scanner is open. */
        bool PortOpen (void);

        /** @brief Retrieve a set of range readings from the scanner. Ranges are returned in millimetres.

        @param readings Pointer to a @ref urg_laser_readings_t structure to store the data in.
        @param min_i The minimum scan index to retrieve.
        @param max_i The maximum scan index to retrieve. */
        int GetReadings (urg_laser_readings_t *readings, int min_i, int max_i);

        /** @brief Get the laser scanner identification information. */
        int GetIDInfo (void);

        /** @brief Get the laser scanner configuration (resolution, scan angles, etc.)

        @param cfg Pointer to a @ref urg_laser_config_t structure to store the configuration in. */
        int GetSensorConfig (urg_laser_config_t *cfg);

        /** @brief Get the protocol version used by the connected laser scanner. */
        int GetSCIPVersion (void);
    private:
        /** @internal SCIP protocol version in use. */
        int SCIP_Version;

        /** @internal File structure of the port connected to the laser scanner. */
        FILE *laser_port;
};

}; // namespace urglaser

/** @} */
