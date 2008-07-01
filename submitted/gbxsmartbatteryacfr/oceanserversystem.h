/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#ifndef GBX_OCEANSERVER_SYSTEM_H
#define GBX_OCEANSERVER_SYSTEM_H

#include <map>
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
        
        //! Read access to all batteries
        const std::map<int,SmartBattery>& batteries() const;
        
        //! Easy write access to single battery, instantiates a new one if it doesn't exist
        SmartBattery& battery( unsigned int batteryNumber );   
        
        //! Easy read access to single battery, battery must exist
        const SmartBattery& battery( unsigned int batteryNumber ) const;
        
        //! Erase a battery
        void eraseBattery( unsigned int batteryNumber );
        
        //! Average battery values
        int percentCharge;
        int minToEmpty;
        std::string messageToSystem;
        
        //! Battery module states
        //! Each vector is always of size 8 because oceanserver system has 8 slots
        std::vector<bool> availableBatteries;
        std::vector<bool> chargingStates;
        std::vector<bool> supplyingPowerStates;
        std::vector<bool> chargePowerPresentStates;
        std::vector<bool> powerNoGoodStates;
        std::vector<bool> chargeInhibitedStates;
 
    private:
        
        // key: slot number, data: a single smart battery module
        std::map<int,SmartBattery> batteries_;
};

//! Puts OceanServerSystem data into a human-readable string
std::string toString( const OceanServerSystem &system );

//! Puts OceanServerSystem data into a machine-readable ASCII string
std::string toLogString( const OceanServerSystem &system );
    
//! Updates all fields in 'to' with data from 'from'. Also reaps batteries in 'to' if they are not in 'from'.
//! Has persistence capabilities: if fields in 'from' are not set and corresponding fields in 'to' are set, the ones in 'to' are kept.
//! Use case: a class stores 'to' as a member variable, receives the latest records into 'from', calls this function to update 'to'.
//! The reaping capability makes sure that battery modules which are no longer connected don't persist.
void updateWithNewData( const OceanServerSystem &from, 
                        OceanServerSystem       &to );
                        
} // namespace

#endif
