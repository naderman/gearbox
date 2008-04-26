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
#include <gbxgarminacfr/driver.h>
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
    // defaults
    string port = "/dev/ttyS0";

    // Get some options from the command line
    while ((opt = getopt(argc, argv, "p:b:")) != -1)
    {
        switch ( opt )
        {
        case 'p':
            port = optarg;
            break;
        default:
            cout << "Usage: " << argv[0] << " [-p port]" << endl << endl
                 << "-p port\tPort the laser scanner is connected to. E.g. /dev/ttyS0" << endl;
            return 1;
        }
    }

    // Set up the laser's configuration
    gbxgarminacfr::Config config;
    config.device = port;
    if ( !config.isValid() ) {
        cout << "Test: Invalid device configuration structure: " << config.toString() << endl;
        exit(1);
    }
    cout << "Using configuration: " << config.toString() << endl;

    // Instantiate objects to handle messages from the driver
    const bool debug=false;
    gbxsickacfr::gbxutilacfr::TrivialTracer tracer( debug );
    gbxsickacfr::gbxutilacfr::TrivialStatus status( tracer );

    // Instantiate the driver itself
    gbxgarminacfr::Driver* device;
    try 
    {
        device = new gbxgarminacfr::Driver( config, tracer, status );
    }
    catch ( const std::exception& e )
    {
        cout <<"Test: Failed to init device: "<<e.what() << endl;
        return 1;
    }

    // Create data structure to store sensor data
    gbxgarminacfr::Data data;

    // Read a few times
    const int numReads = 3;
    for ( int i=0; i < numReads; i++ )
    {
        try 
        {
            device->read( data );

            cout<<"Test: Got data "<<i+1<<" of "<<numReads<<endl;
        }
        catch ( const std::exception& e )
        {
            cout <<"Test: Failed to read data: "<<e.what()<<endl;
        }    
    }

    delete device;
    return 0;
}
