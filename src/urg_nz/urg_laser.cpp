/*
 * Hokuyo URG laser scanner driver class.
 * Originally written by Toby Collett for the Player project.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <poll.h>
#include <sstream>

#include "urg_nz.h"

using namespace urg_nz;
using namespace std;

#ifndef M_PI
    #define M_PI        3.14159265358979323846
#endif
// Convert radians to degrees
#ifndef RTOD
    #define RTOD(r) ((r) * 180 / M_PI)
#endif
// Convert degrees to radians
#ifndef DTOR
    #define DTOR(d) ((d) * M_PI / 180)
#endif

///////////////////////////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////////////////////////
urg_nz::urg_nz (void)
{
    // Defaults to SCIP version 1
    SCIP_Version = 1;
    laser_port   = NULL;
    verbose      = false;
    poll_timeout = -1;
}

///////////////////////////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////////////////////////
urg_nz::~urg_nz (void)
{
    if (PortOpen ())
        Close ();
}

///////////////////////////////////////////////////////////////////////////////
// Read functions
///////////////////////////////////////////////////////////////////////////////
int urg_nz::ReadUntil_nthOccurence (int file, int n, char c)
{
    int retval = 0, bytes_read = 0;
    unsigned char buffer[2];
    buffer[0] = 0;
    buffer[1] = 0;
    for (int i = 0; i < n; i++)
    {
        do
        {
            retval = ReadUntil (file, &buffer[0], 1, poll_timeout);
            if (retval > 0)
                bytes_read += retval;
        } while (buffer[0] != c && retval > 0);
    }
    return bytes_read;
}

///////////////////////////////////////////////////////////////////////////////
int urg_nz::ReadUntil (int fd, unsigned char *buf, int len, int timeout)
{
    int ret;
    int current=0;
    struct pollfd ufd[1];
    int retval;

    ufd[0].fd = fd;
    ufd[0].events = POLLIN;

    do
    {
        if(timeout >= 0)
        {
            if ((retval = poll (ufd, 1, timeout)) < 0)
            {
                stringstream error_desc;
                error_desc << "Error in ReadUntil at poll(): " << errno << ":" << strerror (errno);
                throw urg_nz_exception (URG_ERR_READPOLL, error_desc.str ());
            }
            else if (retval == 0)
            {
                stringstream error_desc;
                error_desc << "Timed out on poll in ReadUntil";
                throw urg_nz_exception (URG_ERR_READTIMEOUT, error_desc.str ());
            }
        }

        ret = read (fd, &buf[current], len-current);
        if (ret < 0)
        {
            stringstream error_desc;
            error_desc << "Error in ReadUntil at read(): " << errno << ":" << strerror (errno);
            throw urg_nz_exception (URG_ERR_READ, error_desc.str ());
        }

        current += ret;
        if (current > 2 && current < len && buf[current-2] == '\n' && buf[current-1] == '\n')
        {
            stringstream error_desc;
            error_desc << "Got an end of command while waiting for more data.";
            throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
        }
    } while (current < len);
    return current;
}

///////////////////////////////////////////////////////////////////////////////
// Port control functions
///////////////////////////////////////////////////////////////////////////////
int urg_nz::ChangeBaud (int curr_baud, int new_baud)
{
    struct termios newtio;
    int fd;
    fd = fileno (laser_port);
    int new_baud_constant;

    switch (curr_baud)
    {
        case 19200:
            curr_baud = B19200;
            break;
        case 57600:
            curr_baud = B57600;
            break;
        case 115200:
            curr_baud = B115200;
            break;
        default:
            stringstream error_desc;
            error_desc << "Unknown current baud rate: " << curr_baud;
            throw urg_nz_exception (URG_ERR_BADBAUDRATE, error_desc.str ());
    }

    switch (new_baud)
    {
        case 19200:
            new_baud_constant = B19200;
            break;
        case 57600:
            new_baud_constant = B57600;
            break;
        case 115200:
            new_baud_constant = B115200;
            break;
        default:
            stringstream error_desc;
            error_desc << "Unknown new baud rate: " << new_baud;
            throw urg_nz_exception (URG_ERR_BADBAUDRATE, error_desc.str ());
    }

    if (tcgetattr (fd, &newtio) < 0)
    {
        close (fd);
        stringstream error_desc;
        error_desc << "Error in ChangeBaud at tcgetattr: " << errno << ":" << strerror (errno);
        throw urg_nz_exception (URG_ERR_CHANGEBAUD, error_desc.str ());
    }

    cfmakeraw (&newtio);
    cfsetispeed (&newtio, curr_baud);
    cfsetospeed (&newtio, curr_baud);

    if (tcsetattr (fd, TCSAFLUSH, &newtio) < 0 )
    {
        close (fd);
        stringstream error_desc;
        error_desc << "Error in ChangeBaud at tcsetattr: " << errno << ":" << strerror (errno);
        throw urg_nz_exception (URG_ERR_CHANGEBAUD, error_desc.str ());
    }

    unsigned char buf[17];
    memset (buf,0,sizeof (buf));

    // TODO: Check if this works with SCIP2.0
    if (SCIP_Version == 1)
    {
        buf[0] = 'S';
        switch (new_baud_constant)
        {
        case B19200:
            buf[1] = '0';
            buf[2] = '1';
            buf[3] = '9';
            buf[4] = '2';
            buf[5] = '0';
            buf[6] = '0';
            break;
        case B57600:
            buf[1] = '0';
            buf[2] = '5';
            buf[3] = '7';
            buf[4] = '6';
            buf[5] = '0';
            buf[6] = '0';
            break;
        case B115200:
            buf[1] = '1';
            buf[2] = '1';
            buf[3] = '5';
            buf[4] = '2';
            buf[5] = '0';
            buf[6] = '0';
            break;
        default:
            // Already checked above
            break;
        }
        buf[7] = '0';
        buf[8] = '0';
        buf[9] = '0';
        buf[10] = '0';
        buf[11] = '0';
        buf[12] = '0';
        buf[13] = '0';
        buf[14] = '\n';
    }
    else                // SCIP 2
    {
        buf[0] = 'S';
        buf[1] = 'S';
        switch (new_baud)
        {
        case B19200:
            buf[2] = '0';
            buf[3] = '1';
            buf[4] = '9';
            buf[5] = '2';
            buf[6] = '0';
            buf[7] = '0';
            break;
        case B57600:
            buf[2] = '0';
            buf[3] = '5';
            buf[4] = '7';
            buf[5] = '6';
            buf[6] = '0';
            buf[7] = '0';
            break;
        case B115200:
            buf[2] = '1';
            buf[3] = '1';
            buf[4] = '5';
            buf[5] = '2';
            buf[6] = '0';
            buf[7] = '0';
            break;
        default:
            // Already checked above
            break;
        }
        buf[8] = '\n';
    }

    fprintf (laser_port, "%s", buf);
    memset (buf, 0, sizeof (buf));
    int len;
    // The docs say that the response ends in 'status LF LF', where
    // status is '0' if everything went alright.  But it seems that
    // the response actually ends in 'LF status LF'.
    if (((len = ReadUntil (fd, buf, sizeof (buf), poll_timeout)) < 0) ||
        (buf[15] != '0'))
    {
        if (verbose)
            fprintf (stderr, "urg_nz: W: Failed to change baud rate to %d\n", new_baud);
        return -1;
    }
    else
    {
        if (tcgetattr (fd, &newtio) < 0)
        {
            close (fd);
            stringstream error_desc;
            error_desc << "Error in ChangeBaud at tcgetattr: " << errno << ":" << strerror (errno);
            throw urg_nz_exception (URG_ERR_CHANGEBAUD, error_desc.str ());
        }
        cfmakeraw (&newtio);
        cfsetispeed (&newtio, new_baud_constant);
        cfsetospeed (&newtio, new_baud_constant);
        if (tcsetattr (fd, TCSAFLUSH, &newtio) < 0 )
        {
            close (fd);
            stringstream error_desc;
            error_desc << "Error in ChangeBaud at tcsetattr: " << errno << ":" << strerror (errno);
            throw urg_nz_exception (URG_ERR_CHANGEBAUD, error_desc.str ());
        }
        else
        {
            usleep (200000);
        }
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
void urg_nz::Open (const char *port_name, bool use_serial, int baud)
{
    if (PortOpen ())
        this->Close ();

    laser_port = fopen (port_name, "r+");
    if (laser_port == NULL)
    {
        stringstream error_desc;
        error_desc << "Failed to open port " << port_name << " << with error " << errno << ":" << strerror (errno);
        throw urg_nz_exception (URG_ERR_OPEN_FAILED, error_desc.str ());
    }

    int fd = fileno (laser_port);
    if (use_serial)
    {
        if (verbose)
        {
            fprintf (stderr, "urg_nz: I: Connecting using serial connection to %s\n", port_name);
            fprintf (stderr, "urg_nz: I: Trying to connect at 19200\n");
        }
        if (this->ChangeBaud (19200, baud) != 0)
        {
            if (verbose)
                fprintf (stderr, "urg_nz: I: Trying to connect at 57600\n");
            if (this->ChangeBaud (57600, baud) != 0)
            {
                if (verbose)
                    fprintf (stderr, "urg_nz: I: Trying to connect at 115200\n");
                if (this->ChangeBaud (115200, baud) != 0)
                {
                    close (fd);
                    stringstream error_desc;
                    error_desc << "Failed to connect at any baud";
                    throw urg_nz_exception (URG_ERR_CONNECT_FAILED, error_desc.str ());
                }
            }
        }
        if (verbose)
            fprintf (stderr, "urg_nz: I: Successfully changed baud rate\n");
    }
    else
    {
        if (verbose)
            fprintf (stderr, "urg_nz: I: Connecting using USB connection to %s\n", port_name);
        // set up new settings
        struct termios newtio;
        memset (&newtio, 0, sizeof (newtio));
        newtio.c_cflag = /*(rate & CBAUD) |*/ CS8 | CLOCAL | CREAD;
        newtio.c_iflag = IGNPAR;
        newtio.c_oflag = 0;
        newtio.c_lflag = ICANON;

        // activate new settings
        tcflush (fd, TCIFLUSH);
        tcsetattr (fd, TCSANOW, &newtio);
        usleep (200000);
        GetSCIPVersion ();
        tcflush (fd, TCIOFLUSH);
    }
}

///////////////////////////////////////////////////////////////////////////////
void urg_nz::Close (void)
{
    assert (this->laser_port);

    if (verbose)
        fprintf (stderr, "urg_nz: I: Closing port\n");

    tcflush (fileno (this->laser_port), TCIOFLUSH);
    if (fclose (this->laser_port) != 0)
    {
        stringstream error_desc;
        error_desc << "Error closing port: " << errno << ":" << strerror (errno);
        this->laser_port = NULL;
        throw urg_nz_exception (URG_ERR_CLOSE_FAILED, error_desc.str ());
    }
    this->laser_port = NULL;
}

///////////////////////////////////////////////////////////////////////////////
bool urg_nz::PortOpen (void)
{
    return laser_port != NULL;
}

///////////////////////////////////////////////////////////////////////////////
// Laser sensor access functions
///////////////////////////////////////////////////////////////////////////////
unsigned int urg_nz::GetReadings (urg_nz_laser_readings_t *readings, unsigned int min_i, unsigned int max_i)
{
    unsigned char buffer[16];
    unsigned int num_readings_read = 0;

    if (readings == NULL)
    {
        stringstream error_desc;
        error_desc << "NULL destination buffer";
        throw urg_nz_exception (URG_ERR_NODESTINATION, error_desc.str ());
    }

    if (!PortOpen ())
    {
        stringstream error_desc;
        error_desc << "Laser port is not open";
        throw urg_nz_exception (URG_ERR_NOPORT, error_desc.str ());
    }

    if (SCIP_Version == 1)
    {
        tcflush (fileno (laser_port), TCIFLUSH);
        // send the command
        fprintf (laser_port, "G00076801\n");

        int file = fileno (laser_port);

        // check the returned command
        ReadUntil (file, buffer, 10, poll_timeout);

        if (strncmp ((const char *) buffer, "G00076801", 9) != 0)
        {
            tcflush (fileno (laser_port), TCIFLUSH);
            stringstream error_desc;
            error_desc << "Error reading command result: " << buffer;
            throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
        }

        // check the returned status
        ReadUntil (file, buffer, 2, poll_timeout);

        if (buffer[0] != '0')
        {
            stringstream error_desc;
            error_desc << "Error reported by laser scanner: " << buffer[0] - '0';
            throw urg_nz_exception (URG_ERR_LASERERROR, error_desc.str ());
        }

        for (unsigned int i=0; ; ++i)
        {
            ReadUntil (file, buffer, 2, poll_timeout);

            if (buffer[0] == '\n' && buffer[1] == '\n')
                break;
            else if (buffer[0] == '\n')
            {
                buffer[0] = buffer[1];
                ReadUntil (file, &buffer[1], 1, poll_timeout);
            }

            if (i < MAX_READINGS)
            {
                readings->Readings[i] = ((buffer[0]-0x30) << 6) | (buffer[1]-0x30);
                num_readings_read++;
            }
            else
            {
                stringstream error_desc;
                error_desc << "Got too many readings: " << i;
                throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
            }
        }

        // Shift the range readings down by min_i if necessary
        if (min_i > 0)
        {
            memmove (&readings->Readings[0], &readings->Readings[min_i], (max_i - min_i) * sizeof (readings->Readings[0]));
            // Don't forget to adjust the number of readings to account for this
            num_readings_read -= (MAX_READINGS - max_i) + min_i;
        }
    }
    else // SCIP_Version == 2
    {
        tcflush (fileno (laser_port), TCIFLUSH);
        // send the command
        fprintf (laser_port, "GD0000076801\n");

        int file = fileno (laser_port);

        // check the returned command
        ReadUntil (file, buffer, 13, poll_timeout);

        if (strncmp ((const char *) buffer, "GD0000076801", 12) != 0)
        {
            tcflush (fileno (laser_port), TCIFLUSH);
            stringstream error_desc;
            error_desc << "Error reading command result: " << buffer;
            throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
        }

        // check the returned status
        ReadUntil (file, buffer, 3, poll_timeout);
        buffer[2] = 0;
        if (buffer[0] != '0' || buffer[1] != '0')
        {
            stringstream error_desc;
            error_desc << "Error reported by laser scanner: " << (buffer[0] - '0') * 10 + (buffer[1] - '0');
            throw urg_nz_exception (URG_ERR_LASERERROR, error_desc.str ());
        }

        ReadUntil_nthOccurence (file, 2, (char)0xa);

        // NOTE: This only works for 769 requested samples.. (64 data bytes
        // blocks are not the best choice for 3-byte values...)

        for (unsigned int i = 0; ; ++i)
        {
            ReadUntil (file, buffer, 3, poll_timeout);

            if ((buffer[1] == '\n') && (buffer[2] == '\n'))
                break;
            else if (buffer[2] == '\n')
                ReadUntil (file, &buffer[1], 2, poll_timeout);
            else if (buffer[0] == '\n')
            {
                if (i <= MAX_READINGS)
                {
                    readings->Readings[i - 1] = ((readings->Readings[i - 1] & 0xFFC0) | (buffer[1]-0x30));
                    buffer [0] = buffer [2];
                    ReadUntil (file, &buffer[1], 2, poll_timeout);
                }
                else
                {
                    stringstream error_desc;
                    error_desc << "Got too many readings: " << i;
                    throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
                }
            }
            else if (buffer[1] == '\n')
            {
                buffer[0] = buffer[2];
                ReadUntil (file, &buffer[1], 2, poll_timeout);
            }

            if (i < MAX_READINGS)
            {
                readings->Readings[i] = ((buffer[0]-0x30) << 12) | ((buffer[1]-0x30) << 6) | (buffer[2]-0x30);
                num_readings_read++;
                if ((readings->Readings[i] > 5600) && (i >= min_i) && (i <= max_i))
                {
                    stringstream error_desc;
                    error_desc << "Reading " << i << " value of " << readings->Readings[i] << " is bigger than 5.6 metres.";
                    throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
                }
            }
            else
            {
                stringstream error_desc;
                error_desc << "Got too many readings: " << i;
                throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
            }
        }

        // Shift the range readings down by min_i if necessary
        if (min_i > 0)
        {
            memmove (&readings->Readings[0], &readings->Readings[min_i], (max_i - min_i) * sizeof (readings->Readings[0]));
            // Don't forget to adjust the number of readings to account for this
            num_readings_read -= (MAX_READINGS - max_i) + min_i;
        }
    }

    return num_readings_read;
}

//////////////////////////////////////////////////////////////////////////////
int urg_nz::GetIDInfo (void)
{
    unsigned char buffer [18];
    memset (buffer, 0, 18);
    int i;
    int id;

    if (!PortOpen ())
    {
        stringstream error_desc;
        error_desc << "Laser port is not open";
        throw urg_nz_exception (URG_ERR_NOPORT, error_desc.str ());
    }

    tcflush (fileno (laser_port), TCIFLUSH);

    if (SCIP_Version == 1)
    {
        // send the command
        fprintf (laser_port, "V\n");

        int file = fileno (laser_port);

        // check the returned command
        ReadUntil (file, buffer, 2, poll_timeout);

        if (strncmp ((const char *) buffer, "V", 1) != 0)
        {
            tcflush (fileno (laser_port), TCIFLUSH);
            stringstream error_desc;
            error_desc << "Error reading command result: " << buffer;
            throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
        }

        // check the returned status
        ReadUntil (file, buffer, 2, poll_timeout);

        if (buffer[0] != '0')
        {
            stringstream error_desc;
            error_desc << "Error reported by laser scanner: " << buffer[0] - '0';
            throw urg_nz_exception (URG_ERR_LASERERROR, error_desc.str ());
        }

        buffer[0] = 0;
        // Read the rest of the values
        for (i = 0; i < 4; i++)
        {
            do
            {
                ReadUntil (file, &buffer[0], 1, poll_timeout);
            } while (buffer[0] != 0xa);
        }

        // Read "SERI:H"
        ReadUntil (file, buffer, 6, poll_timeout);
        // Read the serial number value
        for (i = 0; ; i++)
        {
            ReadUntil (file, &buffer[i], 1, poll_timeout);
            if (buffer[i] == 0xa)
                break;
        }

        id = atol ((const char*)buffer);
        // Read the last LF
        ReadUntil (file, buffer, 1, poll_timeout);
    }
    else // SCIP_Version == 2
    {
        // send the command
        fprintf (laser_port, "VV\n");

        int file = fileno (laser_port);

        // check the returned command
        ReadUntil (file, buffer, 7, poll_timeout);

        if (strncmp ((const char *) buffer, "VV\n00P\n", 7) != 0)
        {
            tcflush (fileno (laser_port), TCIFLUSH);
            stringstream error_desc;
            error_desc << "Error reading command result: " << buffer;
            throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
        }

        buffer[0] = 0;
        // Read the rest of the values
        for (i = 0; i < 4; i++)
        {
            do
            {
                ReadUntil (file, &buffer[0], 1, poll_timeout);
            } while (buffer[0] != 0xa);
        }

        // Read "SERI:H"
        ReadUntil (file, buffer, 6, poll_timeout);
        // Read the serial number value
        for (i = 0; ; i++)
        {
            ReadUntil (file, &buffer[i], 1, poll_timeout);
            if (buffer[i] == ';')
            {
                buffer[i] = 0;
                break;
            }
        }

        id = atol ((const char*)buffer);

        ReadUntil (file, buffer, 3, poll_timeout);
    }

    return id;
}

///////////////////////////////////////////////////////////////////////////////
void urg_nz::GetSensorConfig (urg_nz_laser_config_t *cfg)
{
    memset (cfg, 0, sizeof (urg_nz_laser_config_t));

    if (SCIP_Version == 1)
    {
        unsigned char buffer[10];
        memset (buffer, 0, 10);
        tcflush (fileno (laser_port), TCIFLUSH);
        // send the command
        fprintf (laser_port, "V\n");

        // Set the default resolution
        cfg->resolution = DTOR (270.0) / static_cast<float> (MAX_READINGS);

        int file = fileno (laser_port);

        // check the returned command
        ReadUntil (file, buffer, 4, poll_timeout);

        if (strncmp ((const char *) buffer, "V\n0\n", 4) != 0)
        {
            tcflush (fileno (laser_port), TCIFLUSH);
            stringstream error_desc;
            error_desc << "Error reading command result: " << buffer;
            throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
        }

        // The following might not work on all versions of the hokuyos
        // since it reads out the Product description returned by 'V'

        ReadUntil_nthOccurence (file, 2, (char)0xa);

        // Read FIRM:
        ReadUntil (file, buffer, 5, poll_timeout);

        if (strncmp ((const char *) buffer, "FIRM:", 5) == 0)
        {
            // Read the firmware version major value
            ReadUntil (file, buffer, 5, poll_timeout);
            buffer[1] = 0;
            int firmware = atol ((const char*)buffer);
            if (verbose)
                fprintf (stderr, "urg_nz: I: Firmware major version is %d\n", firmware);

            if (firmware < 3)
            {
                ReadUntil_nthOccurence (file, 4, (char)0xa);
                tcflush (fileno (laser_port), TCIFLUSH);
                stringstream error_desc;
                error_desc << "Bad firmware version: " << firmware;
                throw urg_nz_exception (URG_ERR_BADFIRMWARE, error_desc.str ());
            }
        }

        ReadUntil_nthOccurence(file, 1, (char)'(');
        ReadUntil_nthOccurence(file, 1, (char)'-');

        int i = 0;
        do
        {
            ReadUntil (file, &buffer[i], 1, poll_timeout);
        } while (buffer[i++] != '[');

        buffer[i-1] = 0;
        int max_range = atol((const char*)buffer);

        ReadUntil_nthOccurence (file, 2, (char)',');
        i = 0;
        do
        {
            ReadUntil(file, &buffer[i], 1, poll_timeout);
        } while (buffer[i++] != '-');

        buffer[i-1] = 0;
        int min_i = atol ((const char*)buffer);
        i = 0;
        do
        {
            ReadUntil (file, &buffer[i], 1, poll_timeout);
        } while (buffer[i++] != '[');
        buffer[i-1] = 0;

        int max_i = atol ((const char*)buffer);

        ReadUntil (file, buffer, 4, poll_timeout);
        if (strncmp ((const char *) buffer, "step", 4) != 0)
        {
            tcflush (fileno (laser_port), TCIFLUSH);
            stringstream error_desc;
            error_desc << "Error reading angle_min_idx and angle_max_idx. Check firmware version.";
            throw urg_nz_exception (URG_ERR_READ, error_desc.str ());
        }
        cfg->max_range  = max_range;
        cfg->min_angle  = (min_i-384)*cfg->resolution;
        cfg->max_angle  = (max_i-384)*cfg->resolution;
        if (verbose)
        {
            fprintf (stderr, "urg_nz: I: URG-04 specifications: [min_angle, max_angle, resolution, max_range] = [%f, %f, %f, %f]\n",
                    cfg->min_angle, cfg->max_angle, cfg->resolution, cfg->max_range);
        }
        tcflush (fileno(laser_port), TCIFLUSH);
    }
    else                        // SCIP_Version = 2
    {
        // ask hokuyo: PP
        unsigned char buffer[10];
        memset (buffer, 0, 10);
        tcflush (fileno (laser_port), TCIFLUSH);

        // send the command
        fprintf (laser_port, "PP\n");

        int file = fileno (laser_port);

        // check the returned command
        ReadUntil (file, buffer,7, poll_timeout);

        if (strncmp ((const char *) buffer, "PP\n00P\n", 7) != 0)
        {
            tcflush (fileno (laser_port), TCIFLUSH);
            stringstream error_desc;
            error_desc << "Error reading command result: " << buffer;
            throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
        }
        int i = 0;
        ReadUntil_nthOccurence (file, 2, (char)0xa);
        // read DMAX
        ReadUntil_nthOccurence (file, 1, ':');
        do
        {
            ReadUntil (file, &buffer[i], 1, poll_timeout);
            i++;
        } while (buffer[i-1] != ';');
        buffer[i-1] = 0;
        cfg->max_range = atol ((const char*)buffer);

        // read angular resolution
        ReadUntil_nthOccurence (file, 1, ':');
        i = 0;
        do
        {
            ReadUntil (file, &buffer[i], 1, poll_timeout);
            i++;
        } while (buffer[i-1] != ';');
        buffer[i-1] = 0;
        cfg->resolution = DTOR (360.0 / atol ((const char*)buffer));

        // read AMIN
        ReadUntil_nthOccurence (file, 1, ':');
        i = 0;
        do
        {
            ReadUntil (file, &buffer[i], 1, poll_timeout);
            i++;
        } while (buffer[i-1] != ';');
        buffer[i-1] = 0;
        cfg->min_angle = atol ((const char*)buffer);
        cfg->min_angle -= 384.0;
        cfg->min_angle *= cfg->resolution;

        // read AMAX
        ReadUntil_nthOccurence (file, 1, ':');
        i=0;
        do
        {
            ReadUntil (file, &buffer[i], 1, poll_timeout);
            i++;
        } while (buffer[i-1] != ';');
        buffer[i-1] = 0;
        cfg->max_angle = atol ((const char*)buffer);
        cfg->max_angle -= 384.0;
        cfg->max_angle *= cfg->resolution;

        ReadUntil_nthOccurence (file, 4, (char)0xa);

        if (verbose)
        {
            fprintf (stderr, "urg_nz: I: URG-04 specifications: [min_angle, max_angle, resolution, max_range] = [%f, %f, %f, %f]\n",
                    cfg->min_angle, cfg->max_angle, cfg->resolution, cfg->max_range);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
int urg_nz::GetSCIPVersion (void)
{
    unsigned char buffer [18];
    memset (buffer, 0, 18);
    int file = fileno (laser_port);
    /////////////////
    // try SCIP1 first:
    /////////////////
    tcflush (fileno (laser_port), TCIFLUSH);
    fprintf (laser_port, "V\n");

    // check the returned command
    memset (buffer, 0, 18);
    ReadUntil (file, buffer, 4, poll_timeout);

    if (strncmp ((const char *) buffer, "V\n0\n", 4) != 0)
    {
        // SCIP1.0 failed, so we test it with SCIP2.0:
        tcflush (fileno (laser_port), TCIFLUSH);
        fprintf (laser_port, "VV\n");

        int file = fileno (laser_port);

        // check the returned command
        memset (buffer, 0, 18);
        ReadUntil (file, buffer, 7, poll_timeout);
        tcflush (fileno (laser_port), TCIFLUSH);

        if (strncmp ((const char *) buffer, "VV\n00P\n", 7) != 0)
        {
            stringstream error_desc;
            error_desc << "Error reading after VV command. Answer: " << buffer;
            throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
        }

        // Set SCIP version 2 and return
        SCIP_Version = 2;
        return SCIP_Version;
    }

    // we are currently in SCIP 1.0
    else
    {
        buffer[0] = 0;
        // Read the rest of the values, up till right before firmware version
        ReadUntil_nthOccurence (file, 2, (char)0xa);
        // Read "FIRM:"
        memset (buffer, 0, 18);
        ReadUntil (file, buffer, 5, poll_timeout);

        if (strncmp ((const char *) buffer, "FIRM:", 5) != 0)
        {
            stringstream error_desc;
            error_desc << "FIRM: is not where it is supposed to be: " << buffer;
            throw urg_nz_exception (URG_ERR_PROTOCOL, error_desc.str ());
        }

        // Read the firmware version major value
        ReadUntil (file, buffer, 5, poll_timeout);
        buffer[1] = 0;
        int firmware = atol ((const char*)buffer);
        if (verbose)
            fprintf (stderr, "urg_nz: I: Firmware major version is %d\n", firmware);

        ReadUntil_nthOccurence (file, 4, (char)0xa);
        if (firmware < 3)
        {
            // Set SCIP version 1 and return
            SCIP_Version = 1;
            return SCIP_Version;
        }
        else
        {
            // try to switch to SCIP2.0
            tcflush (fileno (laser_port), TCIFLUSH);
            fprintf (laser_port, "SCIP2.0\n");

            // check the returned command
            memset (buffer, 0, 18);
            ReadUntil (file, buffer, 2, poll_timeout);
            if (strncmp ((const char *) buffer, "SC", 2) != 0)
            {
                // Set SCIP version 1 and return
                SCIP_Version = 1;
                return SCIP_Version;
            }
            else
            {
                memset (&buffer[2], 0, 16);
                ReadUntil (file, &buffer[2], 8, poll_timeout);
                if (strncmp ((const char *) buffer, "SCIP2.0\n0\n", 11) != 0)
                {
                    // Set SCIP version 1 and return
                    SCIP_Version = 1;
                    return SCIP_Version;
                }
                // Set SCIP version 2, turn laser on and return
                SCIP_Version = 2;
                fprintf (laser_port, "BM\n");
                ReadUntil_nthOccurence (file, 3, (char)0xa);
                tcflush (fileno (laser_port), TCIFLUSH);
                return SCIP_Version;
            }
        }
    }

    // Shouldn't get here
    stringstream error_desc;
    error_desc << "Unknown SCIP version";
    throw urg_nz_exception (URG_ERR_SCIPVERSION, error_desc.str ());
}
