/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <iostream>
#include <sstream>

#include <gbxsmartbatteryacfr/exceptions.h>
#include <gbxsmartbatteryacfr/smartbatteryparsing.h>

#include "oceanserverparser.h"

using namespace std;

namespace gbxsmartbatteryacfr {

//
// Helper functions    
//
string toString( const vector<bool> &flags )
{
    stringstream ss;
    for (unsigned int i=0; i<flags.size(); i++)
    {
        ss << flags[i] << " ";
    }
    return ss.str();
}

string toLogString( const vector<bool> &flags )
{
    stringstream ss;
    for (unsigned int i=0; i<flags.size(); i++)
    {
        ss << flags[i];
    }
    return ss.str();
}
    
    
//
// OceanServerSystem non-member functions
//

string toString( const OceanServerSystem &system )
{
    stringstream ss;
    ss << "Charge:          \t" << system.percentCharge << endl;
    ss << "Minutes to empty:\t" << system.minToEmpty << endl;
    ss << "Available batt.: \t" << toString( system.availableBatteries ) << endl;
    ss << "Charging:        \t" << toString( system.chargingStates ) << endl;
    ss << "Supplying power: \t" << toString( system.supplyingPowerStates ) << endl;
    ss << "Charge power:    \t" << toString( system.chargePowerPresentStates ) << endl;
    ss << "Power no good:   \t" << toString( system.powerNoGoodStates ) << endl;
    ss << "Charge inhibited:\t" << toString( system.chargeInhibitedStates ) << endl;
    
    map<int,SmartBattery>::const_iterator it;
    for (it=system.batteries().begin(); it!=system.batteries().end(); it++)
    {
        ss << "Data from battery number: " << it->first << endl;
        ss << toString( it->second ) << endl;
    }

    return ss.str();
}
    
string toLogString( const OceanServerSystem &system )
{
    stringstream ss;
    ss << system.percentCharge << " ";
    ss << system.minToEmpty << " ";
    ss << toLogString( system.availableBatteries ) << " ";
    ss << toLogString( system.chargingStates ) << " ";
    ss << toLogString( system.supplyingPowerStates ) << " ";
    ss << toLogString( system.chargePowerPresentStates ) << " ";
    ss << toLogString( system.powerNoGoodStates ) << " ";
    ss << toLogString( system.chargeInhibitedStates ) << endl;
    
    ss << system.batteries().size();
    
    map<int,SmartBattery>::const_iterator it;
    for (it=system.batteries().begin(); it!=system.batteries().end(); it++)
    {
        ss << it->first << " " << toLogString( it->second ) << endl;
    }

    return ss.str();
}
    
void updateWithNewData( const OceanServerSystem &from, 
                        OceanServerSystem       &to )
{
    typedef map<int,SmartBattery>::const_iterator BatIt;
    
    to.availableBatteries = from.availableBatteries;
    to.percentCharge = from.percentCharge;
    to.minToEmpty = from.minToEmpty;
    to.messageToSystem = from.messageToSystem;
    to.chargingStates = from.chargingStates;
    to.supplyingPowerStates = from.supplyingPowerStates;
    to.chargePowerPresentStates = from.chargePowerPresentStates;
    to.powerNoGoodStates = from.powerNoGoodStates;
    to.chargeInhibitedStates = from.chargeInhibitedStates;
    
    for (BatIt it=from.batteries().begin(); it!=from.batteries().end(); it++)
    {        
        const SmartBattery &fromB = from.battery( it->first );
        SmartBattery &toB = to.battery( it->first ); 
        
        if ( fromB.has( ManufacturerAccess  ) ) toB.setManufacturerAccess ( fromB.manufacturerAccess () );
        if ( fromB.has( RemainingCapacityAlarm  ) ) toB.setRemainingCapacityAlarm ( fromB.remainingCapacityAlarm () );
        if ( fromB.has( RemainingTimeAlarm  ) ) toB.setRemainingTimeAlarm ( fromB.remainingTimeAlarm () );
        if ( fromB.has( BatteryMode  ) ) toB.setBatteryMode ( fromB.batteryMode () );
        if ( fromB.has( AtRate  ) ) toB.setAtRate ( fromB.atRate () );
        if ( fromB.has( AtRateTimeToFull  ) ) toB.setAtRateTimeToFull ( fromB.atRateTimeToFull () );
        if ( fromB.has( AtRateTimeToEmpty  ) ) toB.setAtRateTimeToEmpty ( fromB.atRateTimeToEmpty () );
        if ( fromB.has( AtRateOk  ) ) toB.setAtRateOk ( fromB.atRateOk () );
        if ( fromB.has( Temperature  ) ) toB.setTemperature ( fromB.temperature () );
        if ( fromB.has( Voltage  ) ) toB.setVoltage ( fromB.voltage () );
        if ( fromB.has( Current  ) ) toB.setCurrent ( fromB.current () );
        if ( fromB.has( AverageCurrent  ) ) toB.setAverageCurrent ( fromB.averageCurrent () );
        if ( fromB.has( MaxError  ) ) toB.setMaxError ( fromB.maxError () );
        if ( fromB.has( RelativeStateOfCharge  ) ) toB.setRelativeStateOfCharge ( fromB.relativeStateOfCharge () );
        if ( fromB.has( AbsoluteStateOfCharge  ) ) toB.setAbsoluteStateOfCharge ( fromB.absoluteStateOfCharge () );
        if ( fromB.has( RemainingCapacity  ) ) toB.setRemainingCapacity ( fromB.remainingCapacity () );
        if ( fromB.has( FullChargeCapacity  ) ) toB.setFullChargeCapacity ( fromB.fullChargeCapacity () );
        if ( fromB.has( RunTimeToEmpty  ) ) toB.setRunTimeToEmpty ( fromB.runTimeToEmpty () );
        if ( fromB.has( AverageTimeToEmpty  ) ) toB.setAverageTimeToEmpty ( fromB.averageTimeToEmpty () );
        if ( fromB.has( AverageTimeToFull  ) ) toB.setAverageTimeToFull ( fromB.averageTimeToFull () );
        if ( fromB.has( ChargingCurrent  ) ) toB.setChargingCurrent ( fromB.chargingCurrent () );
        if ( fromB.has( ChargingVoltage  ) ) toB.setChargingVoltage ( fromB.chargingVoltage () );
        if ( fromB.has( BatteryStatus  ) ) toB.setBatteryStatus ( fromB.batteryStatus () );
        if ( fromB.has( CycleCount  ) ) toB.setCycleCount ( fromB.cycleCount () );
        if ( fromB.has( DesignCapacity  ) ) toB.setDesignCapacity ( fromB.designCapacity () );
        if ( fromB.has( DesignVoltage  ) ) toB.setDesignVoltage ( fromB.designVoltage () );
        if ( fromB.has( SpecificationInfo  ) ) toB.setSpecificationInfo ( fromB.specificationInfo () );
        if ( fromB.has( ManufactureDate  ) ) toB.setManufactureDate ( fromB.manufactureDate () );
        if ( fromB.has( SerialNumber  ) ) toB.setSerialNumber ( fromB.serialNumber () );
        if ( fromB.has( ManufacturerName  ) ) toB.setManufacturerName ( fromB.manufacturerName () );
        if ( fromB.has( DeviceName  ) ) toB.setDeviceName ( fromB.deviceName () );
        if ( fromB.has( DeviceChemistry  ) ) toB.setDeviceChemistry ( fromB.deviceChemistry () );
        if ( fromB.has( ManufacturerData  ) ) toB.setManufacturerData ( fromB.manufacturerData () );
    }
    
    // check if reaping needs to be done
    if ( from.batteries().size() == to.batteries().size() ) 
        return;
  
    // store batteries to be reaped in a vector
    vector<int> reapingIds;

    // go through all 'to' batteries and check if they are also in 'from'
    for (BatIt it=to.batteries().begin(); it!=to.batteries().end(); it++)
    {
        const int batId = it->first;
        
        BatIt itFrom = from.batteries().find( batId );
        if ( itFrom == from.batteries().end() ) {
            // battery is in 'to' but not in 'from' -> needs to be deleted
            reapingIds.push_back( batId );
        }
    }
    
    // erase batteries
    for (unsigned int i=0; i<reapingIds.size(); i++) {
        to.eraseBattery( reapingIds[i] );
    }
    
}


//
// OceanServerSystem member functions
//

OceanServerSystem::OceanServerSystem()
    : percentCharge(0),
      minToEmpty(0),
      messageToSystem("")
{
    // fixed number of slots for oceanserver system
    const int NUM_BATTERY_SLOTS = 8;
    availableBatteries.resize(NUM_BATTERY_SLOTS);
    chargingStates.resize(NUM_BATTERY_SLOTS);
    supplyingPowerStates.resize(NUM_BATTERY_SLOTS);
    chargePowerPresentStates.resize(NUM_BATTERY_SLOTS);
    powerNoGoodStates.resize(NUM_BATTERY_SLOTS);
    chargeInhibitedStates.resize(NUM_BATTERY_SLOTS);
}

// read access to all batteries
const map<int,SmartBattery>&
OceanServerSystem::batteries() const
{ 
    return batteries_; 
}

// write access to single battery
SmartBattery& 
OceanServerSystem::battery( unsigned int batteryNumber )
{
    map<int,SmartBattery>::iterator it = batteries_.find(batteryNumber);
    if ( it==batteries_.end() )
    {
        // we don't have it, so instantiate a new one
        SmartBattery b;
        batteries_[batteryNumber] = b;
        return batteries_[batteryNumber];
    }
    
    return it->second;
}  

// read access to single battery
const SmartBattery& 
OceanServerSystem::battery( unsigned int batteryNumber ) const
{
    map<int,SmartBattery>::const_iterator it = batteries_.find(batteryNumber);
    if ( it==batteries_.end() )
    {
        stringstream ss; 
        ss << "ERROR(OceanServerParser.cpp): trying to read from non-existent battery " << batteryNumber;
        throw ParsingException( ss.str().c_str() );
    }
    return it->second;
}

void
OceanServerSystem::eraseBattery( unsigned int batteryNumber )
{
    batteries_.erase( batteries_.find( batteryNumber ) );    
}

//
//  OceanServerParser member functions
//

OceanServerParser::OceanServerParser( gbxutilacfr::Tracer &tracer )
    : tracer_(tracer)
{
}

void 
OceanServerParser::parseSystemData( const map<string,string> &keyValuePairs,
                                    OceanServerSystem        &batterySystem )
{   
    map<string,string>::const_iterator it;
    
    for (it=keyValuePairs.begin(); it!=keyValuePairs.end(); it++)
    {   
        if (it->first=="01") {
            batterySystem.minToEmpty = readMinutes(it->second);
        }
        else if (it->first=="02") {
            // reserved, do nothing
        }
        else if (it->first=="03") {
            batterySystem.messageToSystem = it->second;
        }
        else if (it->first=="04") {
            batterySystem.percentCharge = readPercentByte(it->second);
        }
        else 
        {
            stringstream ss;
            ss << "OceanServerParser: Unknown System key: " << it->first;
            throw ParsingException(ss.str().c_str());
        }
    }
}

void 
OceanServerParser::parseControllerData( const map<string,string> &keyValuePairs,
                                        OceanServerSystem        &batterySystem )
{
    map<string,string>::const_iterator it;
    
    for (it=keyValuePairs.begin(); it!=keyValuePairs.end(); it++)
    { 
        vector<bool> states;
        
        if (it->first=="01") {
            readSingleByte(it->second, states);
            batterySystem.availableBatteries = states;
        }
        else if (it->first=="02") {
            readSingleByte(it->second, states);
            batterySystem.chargingStates = states;
        }
        else if (it->first=="03") {
            readSingleByte(it->second, states);
            batterySystem.supplyingPowerStates = states;
        }
        else if (it->first=="04") {
            // reserved, do nothing
        }
        else if (it->first=="05") {
            readSingleByte(it->second, states);
            batterySystem.chargePowerPresentStates = states;
        }
        else if (it->first=="06") {
            readSingleByte(it->second, states);
            batterySystem.powerNoGoodStates = states;
        }
        else if (it->first=="07") {
            readSingleByte(it->second, states);
            batterySystem.chargeInhibitedStates = states;
        }
        else 
        {
            stringstream ss;
            ss << "OceanServerParser: Unknown controller key: " << it->first;
            throw ParsingException(ss.str().c_str());
        }

    }
    
}

void
OceanServerParser::parseSingleBatteryData( const map<string,string> &keyValuePairs, 
                                           unsigned int              batteryNum, 
                                           OceanServerSystem        &batterySystem )
{
    map<string,string>::const_iterator it;
    
    // get a reference to the battery whose fields we're updating
    SmartBattery &bat = batterySystem.battery( batteryNum );
    
    for (it=keyValuePairs.begin(); it!=keyValuePairs.end(); it++)
    {   
        SmartBatteryDataField smartField = stringToSmartField( it->first );
        
        switch( smartField )
        {
            case ManufacturerAccess: 
                bat.setManufacturerAccess( read16Flags( it->second ) ); break;
            case RemainingCapacityAlarm:
                bat.setRemainingCapacityAlarm( readCapacity( it->second ) ); break;
            case RemainingTimeAlarm:
                bat.setRemainingTimeAlarm( readMinutes( it->second ) ); break;
            case BatteryMode:
                bat.setBatteryMode( read16Flags( it->second ) ); break;
            case AtRate:
                bat.setAtRate( readRate( it->second ) ); break;
            case AtRateTimeToFull:
                bat.setAtRateTimeToFull( readMinutes( it->second ) ); break;
            case AtRateTimeToEmpty:
                bat.setAtRateTimeToEmpty( readMinutes( it->second ) ); break;
            case AtRateOk:
                bat.setAtRateOk( readBool( it->second ) ); break;
            case Temperature: 
                bat.setTemperature( readTemperature( it->second ) ); break;
            case Voltage:
                bat.setVoltage( readVoltage( it->second ) ); break;
            case Current: 
                bat.setCurrent( readCurrent( it->second ) ); break;
            case AverageCurrent:
                bat.setAverageCurrent( readCurrent( it->second ) ); break;
            case MaxError:
                bat.setMaxError( readPercentWord( it->second ) ); break;
            case RelativeStateOfCharge:
                bat.setRelativeStateOfCharge( readPercentWord( it->second ) ); break;
            case AbsoluteStateOfCharge:
                bat.setAbsoluteStateOfCharge( readPercentWord( it->second ) ); break;
            case RemainingCapacity:
                bat.setRemainingCapacity( readCapacity( it->second ) ); break;
            case FullChargeCapacity:
                bat.setFullChargeCapacity( readCapacity( it->second ) ); break;
            case RunTimeToEmpty:
                bat.setRunTimeToEmpty( readMinutes( it->second ) ); break;
            case AverageTimeToEmpty:
                bat.setAverageTimeToEmpty( readMinutes( it->second ) ); break;
            case AverageTimeToFull:
                bat.setAverageTimeToFull( readMinutes( it->second ) ); break;
            case ChargingCurrent:
                bat.setChargingCurrent( readCurrent( it->second ) ); break;
            case ChargingVoltage:
                bat.setChargingVoltage( readVoltage( it->second ) ); break;
            case BatteryStatus:
                bat.setBatteryStatus( read16Flags( it->second ) ); break;
            case CycleCount:
                bat.setCycleCount( readCount (it->second) ); break;
            case DesignCapacity:
                bat.setDesignCapacity( readCapacity( it->second ) ); break;
            case DesignVoltage:
                bat.setDesignVoltage( readVoltage( it->second ) ); break;
            case SpecificationInfo:
                bat.setSpecificationInfo( read16Flags( it->second ) ); break;
            case ManufactureDate:
                bat.setManufactureDate( read16Flags( it->second ) ); break;
            case SerialNumber:
                bat.setSerialNumber( readNumber( it->second ) ); break;
            case ManufacturerName:
                bat.setManufacturerName( it->second ); break;
            case DeviceName:
                bat.setDeviceName( it->second ); break;
            case DeviceChemistry:
                bat.setDeviceChemistry( it->second ); break;
            case ManufacturerData:
                bat.setManufacturerData( read16Flags( it->second ) ); break;
            case NUM_SMARTBATTERY_FIELDS:
            default:
                stringstream ss; ss << "OceanServerParser: Unknown Battery key: " << it->first;
                throw ParsingException( ss.str().c_str() );
        }
    }
    
}

void 
OceanServerParser::parseFields( vector<string>    &fields, 
                                OceanServerSystem &batterySystem )
{
    if (fields.size()==0) return;
    
    // save the msgType string and remove from vector
    string msgType = fields[0];
    vector<string>::iterator it = fields.begin();
    fields.erase( it );
    
    // get the msg type key (S,C,B)
    const string &msgTypeKey = msgType.substr(0,2);
    
    // make key-value pairs
    map<string,string> keyValuePairs;    
    toKeyValuePairs( fields, keyValuePairs, tracer_ );
    
    if (msgTypeKey=="$S") 
    {
        parseSystemData( keyValuePairs, batterySystem );
    } 
    else if  (msgTypeKey=="$C")
    {
        parseControllerData( keyValuePairs, batterySystem );    
    }
    else if (msgTypeKey=="$B")
    {   
        stringstream ss(msgType.substr(3));
        int batteryNum;
        ss >> batteryNum;
        parseSingleBatteryData( keyValuePairs, batteryNum, batterySystem );
    }
    else
    {
        stringstream ss;
        ss << "OceanServerParser: Unknown message type: " << msgTypeKey;
        throw ParsingException(ss.str().c_str());
    }
}

bool 
OceanServerParser::atBeginningOfRecord( const char* line )
{
    vector<string> tokens;
    splitIntoFields( line, tokens, ",");
        
    if (tokens.size()>0) {
        if (tokens[0]!="$S") {
            return false;
        }
    }
    return true;
}

bool
OceanServerParser::atEndOfRecord( const char* line )    
{
    vector<string> tokens;
    splitIntoFields( line, tokens, ",");
    
    if (tokens.size()>0) {
        if (tokens[0]=="$S") return true;
    }
    return false;
}


void
OceanServerParser::parse( vector<string>    &stringList, 
                          OceanServerSystem &batterySystem )
{
    
    //
    // Uncomment for DEBUG information
    //

    stringstream ss;
    ss << "OceanServerParser: Received the following input: " << endl;
    for (unsigned int i=0; i<stringList.size(); i++)
    {
        const string &str = stringList[i];
        ss << i << ": " << str;

        // output in hex
        ss << i << " hex: ";
        for (unsigned k=0; k<str.size(); k++)
        {
            unsigned int charValue = (unsigned int)str[k];
            ss << str[k] << ": " << std::hex << charValue << "  ";
        }
        ss << std::dec;

    }
    ss << endl;
    tracer_.debug( ss.str(), 10 );
    
    for (unsigned int i=0; i<stringList.size(); i++)
    {
        const string &line = stringList[i];

        // check for control characters in the line (sometimes they are accidently inserted)
        // don't check the last 2 characters: they're \0 and \n
        for (unsigned int k=0; k<line.size()-2; k++)
        {
            if ( iscntrl( line[k] ) )
                throw ParsingException("ERROR(oceanserverparser.cpp): Found a control character (binary) in the string!");
        }

        // divide the line into 2 parts: data and checksum (if present)
        vector<string> checksumList;
        splitIntoFields(line, checksumList, "%" );
        if (checksumList.size()==2)
        {
            // we have a checksum, is it correct?
            if (!isChecksumValid( checksumList[0], checksumList[1] ) )
                throw ParsingException("ERROR(oceanserverparser.cpp): Checksum failed!");
        }
                 
        // divide the data into individual fields and parse
        if (checksumList.size()==0)
            throw ParsingException("ERROR(oceanserverparser.cpp): String length is 0");
        vector<string> fields;
        splitIntoFields( checksumList[0], fields, ",");
        parseFields( fields, batterySystem );
    }
}

}

