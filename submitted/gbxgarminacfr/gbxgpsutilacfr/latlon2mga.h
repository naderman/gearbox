/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Matthew Ridley
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

// #include <hydroportability/sharedlib.h>

#ifndef LATLON2MGA_H
#define LATLON2MGA_H

namespace gbxgpsutilacfr {
    
    // enumeration of geod models
//     SOEXPORT enum EGeodModel {
    enum EGeodModel { 
        GM_UNDEFINED=-1,
        GM_WGS84 =0,
        GM_GDA94,
        GM_WGS72,
        GM_AGD84,
        GM_NSWC_9Z2,
        GM_Clarke,
        GM_NoOptions,
        GM_GRS80=GM_GDA94,
        GM_ANS=GM_AGD84 };

    // Geod model
//     SOEXPORT typedef struct {
    typedef struct {
        double a;               // Semi major axis (m)
        double b;               // Semi minor axis (m)
        double f;               // flatening
        double e;               // Eccentricity
        double e2;              // Eccentricity^2
        double A0, A2, A4, A6;  // Meridian Distance calculation parameters
    } TGeoModelData;

    // LatLon_2_MGA convert (lat,lon) in degrees to (Northing, Easting) in meters
//     SOEXPORT void LatLon2MGA( 
    void LatLon2MGA( 
        double lat,
        double lon,
        double& Northing,
        double& Easting,
        int& Zone,
        EGeodModel geodmodel = GM_WGS84);

    // MGA_2_LatLon convert (Northing, Easting) in meters to (lat,lon) in degrees
//     SOEXPORT void MGA2LatLon( 
    void MGA2LatLon( 
        double Northing,
        double Easting,
        int Zone,
        double& lat,
        double& lon,
        EGeodModel geodmodel = GM_WGS84);

} //namespace

#endif
