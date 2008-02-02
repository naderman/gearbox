#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <gearbox/urglaser/urg_laser.h>

#define URG04_MIN_STEP 44
#define URG04_MAX_STEP 725

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

    // Open the laser
    urglaser::urg_laser laser;                  // Laser scanner object
    if (laser.Open (port, useSerial, baud) < 0)
    {
        printf ("Failed to open laser.\n");
        return 1;
    }

    // Get the laser serial number
    int serial = 0;
    if ((serial = laser.GetIDInfo ()) < 0)
    {
        printf ("Failed to get laser serial number.\n");
        return 1;
    }
    printf ("Laser serial number:\t%d\n", serial);

    // Get the laser configuration
    urglaser::urg_laser_config_t config;        // Laser configuration structure
    if (laser.GetSensorConfig (&config) < 0)
    {
        printf ("Failed to get laser configuration.\n");
        return 1;
    }
    printf ("Laser configuration:\n"
            "Min angle: %f\tMax angle: %f\tResolution: %f\tMax range: %f\n",
            config.min_angle, config.max_angle, config.resolution, config.max_range);

    // Calculate the minimum and maximum indices to retrieve
    int minIndex = static_cast<int> (round (384 + config.min_angle / config.resolution));
    int maxIndex = static_cast<int> (round (384 + config.max_angle / config.resolution));
    // Ancient firmware versions need some hard limits set, just in case
    if (minIndex < URG04_MIN_STEP)
        minIndex = URG04_MIN_STEP;
    if (maxIndex > URG04_MAX_STEP)
        maxIndex = URG04_MAX_STEP;

    // Get range readings from the laser
    urglaser::urg_laser_readings_t readings;    // Laser readings structure
    printf ("Getting readings from %d to %d\n", minIndex, maxIndex);
    if (laser.GetReadings (&readings, minIndex, maxIndex) < 0)
    {
        printf ("Failed to get laser scan data.\n");
        return 1;
    }
    printf ("Got %d range readings:\n", maxIndex - minIndex);
    for (unsigned int ii = 0; ii < (maxIndex - minIndex); ii++)
    {
        if (readings.Readings[ii] < 20)         // Values less than 20 indicate no return in the scan
            printf ("%f\t", config.max_range);
        else
            printf ("%f\t", readings.Readings[ii]);
    }
    printf ("\n");

    // Close the laser
    laser.Close ();

    return 0;
}
