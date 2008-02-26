/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

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

//
// Instantiates the laser driver, reads a few scans
//
int main( int argc, char **argv )
{
    int opt;
    int baud = 38400;
    string port = "/dev/ttyS0";

    // Get some options from the command line
    while ((opt = getopt(argc, argv, "p:b:")) != -1)
    {
        switch ( opt )
        {
        case 'p':
            port = optarg;
            break;
        case 'b':
            baud = atoi( optarg );
            break;
        default:
            cout << "Usage: " << argv[0] << " [-p port] [-b baud]" << endl << endl
                 << "-p port\tPort the laser scanner is connected to. E.g. /dev/ttyS0" << endl
                 << "-b baud\tBaud rate to connect at (19200, 57600 or 115200)." << endl;
            return 1;
        }
    }

    // Set up the laser's configuration
    gbxsickacfr::Config config;
    config.minRange = 0.0;
    config.maxRange = 80.0;
    config.fieldOfView = 180.0*DEG2RAD_RATIO;
    config.startAngle = -90.0*DEG2RAD_RATIO;
    config.numberOfSamples = 181;
    config.baudRate = baud;
    config.device = port;
    if ( !config.validate() ) {
        cout << "Test: Invalid laser configuration: " << config.toString() << endl;
    }
    cout << "Using configuration: " << config.toString() << endl;

    // Instantiate objects to handle messages from the driver
    const bool debug=false;
    gbxsickacfr::gbxutilacfr::TrivialTracer tracer( debug );
    gbxsickacfr::gbxutilacfr::TrivialStatus status( tracer );

    // Instantiate the driver itself
    gbxsickacfr::Driver* device;
    try 
    {
        device = new gbxsickacfr::Driver( config, tracer, status );
    }
    catch ( const std::exception& e )
    {
        cout <<"Test: Failed to init device: "<<e.what() << endl;
        return 1;
    }

    // Create data structure to store sensor data
    vector<float>         ranges( config.numberOfSamples );
    vector<unsigned char> intensities( config.numberOfSamples );
    gbxsickacfr::Data data;
    data.ranges      = &(ranges[0]);
    data.intensities = &(intensities[0]);

    // Read a few times
    const int numReads = 3;
    for ( int i=0; i < numReads; i++ )
    {
        try 
        {
            device->read( data );

            cout<<"Test: Got scan "<<i+1<<" of "<<numReads<<endl;
            for ( int i=0; i < config.numberOfSamples; i++ )
            {
                const double angle = config.startAngle + i*config.fieldOfView/(double)(config.numberOfSamples-1);
                cout << "  " << i << ": angle=" << angle*180.0/M_PI
                     << "deg, range=" << data.ranges[i]
                     << ", intensity=" << data.intensities[i] << endl;
            }

            if ( data.haveWarnings )
                cout << "got warnings: " << data.warnings << endl;
        }
        catch ( const std::exception& e )
        {
            cout <<"Test: Failed to read scan: "<<e.what()<<endl;
        }    
    }

    delete device;
    return 0;
}
