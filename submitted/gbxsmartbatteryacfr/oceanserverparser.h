/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef OCEANSERVER_PARSER_H
#define OCEANSERVER_PARSER_H

#include <map>
#include <gbxutilacfr/tracer.h>
#include <gbxsmartbatteryacfr/smartbattery.h>

namespace gbxsmartbatteryacfr
{

//!
//! Class representing the OceanServer battery system data
//! Contains average values of the whole system and values from individual batteries
//!
//! @author Tobias Kaupp
//!
class OceanServerSystem
{
    public:
        
        OceanServerSystem();
        ~OceanServerSystem() {};
        
        // read access to all batteries
        const std::map<int,SmartBattery>& batteries() const;
        
        // easy write access to single battery, instantiates a new one if it doesn't exist
        SmartBattery& battery( unsigned int batteryNumber );   
        
        // easy read access to single battery, battery must exist
        const SmartBattery& battery( unsigned int batteryNumber ) const;
        
        // erase a battery
        void eraseBattery( unsigned int batteryNumber );
        
        // average battery values
        int percentCharge;
        int minToEmpty;
        std::string messageToSystem;
        
        // values from the controller
        // each vector is always of size 8 because oceanserver system has 8 slots
        std::vector<bool> availableBatteries;
        std::vector<bool> chargingStates;
        std::vector<bool> supplyingPowerStates;
        std::vector<bool> chargePowerPresentStates;
        std::vector<bool> powerNoGoodStates;
        std::vector<bool> chargeInhibitedStates;
 
    private:
        std::map<int,SmartBattery> batteries_;
};

//! Puts all available data into a human-readable string
std::string toString( const OceanServerSystem &system );

//! Puts all available data into a machine-readable ASCII string
std::string toLogString( const OceanServerSystem &system );
    
//! Updates all fields in 'to' with data from 'from'. Also reapes batteries in 'to' if they are not in 'from'.
//! Has persistence capabilities: if fields in 'from' are not set and corresponding fields in 'to' are set, the ones in 'to' are kept.
//! Use case: a class stores 'to' as a member variable, receives the latest records into 'from', calls this function to update 'to'.
//! The reaping capability makes sure that battery modules which are no longer connected don't persist.
void updateWithNewData( const OceanServerSystem &from, 
                        OceanServerSystem       &to );
    
//! 
//! Class to parse the hex data the oceanserver battery controller spits out
//!
//! @author Tobias Kaupp
//!
class OceanServerParser
{
public:
    
    OceanServerParser( gbxutilacfr::Tracer &tracer ); 
    ~OceanServerParser() {};
    
    //! Expects a full record of batterydata as a stringList (one line per string) produced by the oceanserver controller.
    //! Parses each line and sets corresponding fields in batterySystem
    void parse( std::vector<std::string> &stringList, 
                OceanServerSystem        &batterySystem );
    
    //! Checks whether the passed string (one line) is the first line of the record
    bool atBeginningOfRecord( const char *string );
    
    //! Checks whether the passed string (one line) is the end of the record
    bool atEndOfRecord( const char *string );

private:       
    
    gbxutilacfr::Tracer &tracer_;
            
    // parsing functions
    void parseFields( std::vector<std::string>       &fields, 
                      OceanServerSystem              &batterySystem );
    
    void parseSystemData( const std::map<std::string,std::string> &keyValuePairs,
                          OceanServerSystem                       &batterySystem);
    
    void parseControllerData( const std::map<std::string,std::string> &keyValuePairs,
                              OceanServerSystem                       &batterySystem);
    
    void parseSingleBatteryData( const std::map<std::string,std::string> &keyValuePairs, 
                                 unsigned int                             batteryNum,
                                 OceanServerSystem                       &batterySystem);
    
};


} // namespace

#endif
