/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Mathew Ridley, Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <gbxgarminacfr/gbxgpsutilacfr/latlon2mga.h>
#include <iostream>
#include <cmath>
#include <cstdlib>

using namespace std;
using namespace gbxgpsutilacfr;

bool close( double a, double b )
{
    if ( fabs(a-b) > 1e-3 )
    {
        cout<<"TRACE(testmga.cpp): diff: " << a-b << endl;
        return false;
    }
    return true;
}

int main()
{
    double northing, easting;

    double lat = -33.8895;
    double lon = 151.193;

    cout<<"TRACE(testmga.cpp): lat, lon: " << lat << ", " << lon << endl;

    int zone;
    LatLon2MGA( lat, lon, northing, easting, zone );
    cout<<"TRACE(testmga.cpp): Using LatLon2MGA: northing,easting = " << northing << ", " << easting << endl;

    double backlat, backlon;
    MGA2LatLon( northing, easting, zone, backlat, backlon );
    cout<<"TRACE(testmga.cpp): Converting back: lat,lon = " << backlat << ", " << backlon << endl;
    
    if ( ! ( close(lat,backlat) && close(lon,backlon) ) )
    {
        cout << "ERROR(testmga.cpp): latlon->mga->latlon is broken." << endl;
        exit(1);
    }

    cout<<"TRACE(testmga.cpp): test PASSED" << endl;
    return 0;
}
