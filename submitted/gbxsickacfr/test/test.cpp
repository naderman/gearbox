#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <gbxsickacfr/driver.h>
#include <gbxsickacfr/gbxutilacfr/trivialtracer.h>
#include <gbxsickacfr/gbxutilacfr/trivialstatus.h>
#include <gbxsickacfr/gbxutilacfr/mathdefs.h>

using namespace std;


int main( int argc, char **argv )
{
    int opt;
//     int baud = 115200;
//     bool useSerial = false;
    string port = "/dev/ttyS0";
    int maxCount = 1;

    // Get some options from the command line
    while ((opt = getopt (argc, argv, "p:c:")) != -1)
    {
        switch (opt)
        {
            case 'p':
//                 strncpy (port, optarg, 256);
                port = optarg;
                break;
            case 'c':
                maxCount = atoi( optarg );
                break;
//             case 'b':
//                 baud = atoi (optarg);
//                 if (baud != 19200 && baud != 57600 && baud != 115200)
//                 {
//                     printf ("Baud rate must be one of 19200, 57600 or 115200.\n");
//                     return 1;
//                 }
//                 break;
//             case 's':
//                 useSerial = true;
//                 break;
            default:
                printf ("Usage: %s [-p port] [-b baud] [-s]\n\n"
                        "-p port\tPort the laser scanner is connected to. E.g. /dev/ttyS0\n"
                        "-b baud\tBaud rate to connect at (19200, 57600 or 115200).\t"
                        "-s\tUse RS232 connection instead of USB.\n", argv[0]);
                return 1;
        }
    }


    gbxsickacfr::Config config;

    config.minRange = 0.0;
    config.maxRange = 80.0;
    config.fieldOfView = 180.0*DEG2RAD_RATIO;
    config.startAngle = -90.0*DEG2RAD_RATIO;
    config.numberOfSamples = 181;
    config.baudRate = 38400;
    config.device = port;

    gbxsickacfr::gbxutilacfr::TrivialTracer tracer;
    gbxsickacfr::gbxutilacfr::TrivialStatus status( tracer );

    if ( !config.validate() ) {
        tracer.error( "Test: Failed to validate laser configuration. "+config.toString() );
    }

    tracer.info( "Validated configuration structure: "+config.toString() );

    // initialize
    gbxsickacfr::Driver* device;
    try 
    {
        device = new gbxsickacfr::Driver( config, tracer, status );
    }
    catch ( const std::exception& e )
    {
        stringstream ss; ss<<"Test: Failed to init device: "<<e.what();
        tracer.error( ss.str() );
        return 1;
    }

    // allocated data storage
    vector<float> ranges;
    vector<unsigned char> intensities;
    ranges.resize( config.numberOfSamples );
    intensities.resize( config.numberOfSamples );

    // create data structure
    gbxsickacfr::Data data;
    data.ranges      = &(ranges[0]);
    data.intensities = &(intensities[0]);

    int count = 0;
    while ( count < maxCount )
    {
        // catch exceptions!!!
        try 
        {
            // using default timeout of 1000ms
            device->read( data );
        }
        catch ( const std::exception& e )
        {
            stringstream ss; ss<<"Test: Failed to read scan: "<<e.what();
            tracer.error( ss.str() );
        }
    
    //         data.timeStampSec;
    //         data.timeStampUsec;

        stringstream ss; ss<<"Test: Got scan "<<count+1<<" of "<<maxCount;
        tracer.info( ss.str() );
    
        if ( data.haveWarnings )
            tracer.warning( "got warnings: "+data.warnings );

        count++;
    }

    return 0;
}
