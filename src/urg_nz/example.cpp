#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gearbox/urglaser/urg_laser.h>

int main (int argc, char **argv)
{
    int opt, baud = 115200;
    bool useSerial = false;
    char port[256];

    // Get some options from the command line
    while ((opt = getopt (argc, argv, "p:b:s")) != -1)
    {
        switch (opt)
        {
            case 'p':
                strncpy (port, optarg, 256);
                break;
            case 'b':
                baud = atoi (optarg);
                if (baud != 19200 && baud != 57600 && baud != 115200)
                {
                    printf ("Baud rate must be one of 19200, 57600 or 115200.\n");
                    return 1;
                }
                break;
            case 's':
                useSerial = true;
                break;
            default:
                printf ("Usage: %s [-p port] [-b baud] [-s]\n\n"
                        "-p port\tPort the laser scanner is connected to.\n"
                        "-b baud\tBaud rate to connect at (19200, 57600 or 115200).\t"
                        "-s\tUse RS232 connection instead of USB.\n", argv[0]);
                return 1;
        }
    }

    try
    {
        urglaser::urg_laser laser;                  // Laser scanner object
        // Set the laser to verbose mode (so we see more information in the console)
        laser.SetVerbose (true);

        // Open the laser
        laser.Open (port, useSerial, baud);

        // Set the timeout to 1000ms
        laser.SetTimeOut (1000);

        // Check the SCIP protocol version
        int version = laser.GetSCIPVersion ();
        printf ("Laser is using SCIP protocol version %d\n", version);

        // Get the laser serial number
        int serial = 0;
        serial = laser.GetIDInfo ();
        printf ("Laser serial number:\t%d\n", serial);

        // Get the laser configuration
        urglaser::urg_laser_config_t config;        // Laser configuration structure
        laser.GetSensorConfig (&config);
        printf ("Laser configuration:\n"
                "Min angle: %f\tMax angle: %f\tResolution: %f\tMax range: %f\n",
                config.min_angle, config.max_angle, config.resolution, config.max_range);

        // Calculate the minimum and maximum indices to retrieve - this is only necessary if you want
        // less than the full scan, otherwise simply don't supply min_i and max_i to urg_laser.GetReadings()
        // and the full scan will be returned.
        int minIndex = static_cast<int> (round ((urglaser::MAX_READINGS / 2) + config.min_angle / config.resolution));
        int maxIndex = static_cast<int> (round ((urglaser::MAX_READINGS / 2) + config.max_angle / config.resolution));

        if (useSerial && version == 1)      // Baud rate changing is not currently supported on SCIP v2 scanners
        {
            // Change the baud rate
            if (laser.ChangeBaud (19200, 57600) == 0)
                printf ("Changed baud rate to 57600.\n");
            else
                printf ("Unable to change baud rate.\n");
        }

        // Get range readings from the laser
        urglaser::urg_laser_readings_t readings;    // Laser readings structure
        printf ("Getting readings from %d to %d\n", minIndex, maxIndex);
        unsigned int numRead = laser.GetReadings (&readings, minIndex, maxIndex);
        printf ("Got %d range readings:\n", numRead);
        for (unsigned int ii = 0; ii < numRead; ii++)
        {
            float angle = config.min_angle + (ii * config.resolution);
            if (readings.Readings[ii] < 20)         // Values less than 20 indicate no return in the scan
                printf ("%f: %f\t\t", angle, config.max_range);
            else
                printf ("%f: %d\t\t", angle, readings.Readings[ii]);
        }
        printf ("\n");

        // Close the laser
        laser.Close ();
    }
    catch (urglaser::urg_exception e)
    {
        printf ("Caught exception: (%d) %s\n", e.error_code, e.error_desc.c_str ());
        return -1;
    }

    return 0;
}
