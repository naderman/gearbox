/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Michael Moser
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
#ifndef GBX_NOVATEL_RECEIVER_STATUS_DECODER_H
#define GBX_NOVATEL_RECEIVER_STATUS_DECODER_H

#include <stdint.h>
#include <string>
#include <sstream>
namespace gbxnovatelutilacfr{
bool receiverStatusIsGood(uint32_t receiverStatus){
    return 0 == receiverStatus & 0xe1fe8fef;
}
//bool receiverStatusIsWarning(uint32_t receiverStatus){
//}
//bool receiverStatusIsError(uint32_t receiverStatus){
//}
std::string receiverStatusToString(uint32_t receiverStatus){
    std::stringstream ss;
    ss << "Error flag: "
        << ((0 == (receiverStatus & 0x00000001)) ? "No error" : "Error") << "; ";
    ss << "Temperature status: "
        << ((0 == (receiverStatus & 0x00000002)) ? "Within specifications" : "Warning") << "; ";
    ss << "Voltage supply status: "
        << ((0 == (receiverStatus & 0x00000004)) ? "OK" : "Warning") << "; ";
    ss << "Antenna power status: "
        << ((0 == (receiverStatus & 0x00000008)) ? "Powered" : "Not powered") << "; ";
    ss << "Antenna open flag: "
        << ((0 == (receiverStatus & 0x00000020)) ? "OK" : "Open") << "; ";
    ss << "Antenna shorted flag: "
        << ((0 == (receiverStatus & 0x00000040)) ? "OK" : "Shorted") << "; ";
    ss << "CPU overload flag: "
        << ((0 == (receiverStatus & 0x00000080)) ? "No overload" : "Overload") << "; ";
    ss << "COM1 buffer overrun flag: "
        << ((0 == (receiverStatus & 0x00000100)) ? "No overrun" : "Overrun") << "; ";
    ss << "COM2 buffer overrun flag: "
        << ((0 == (receiverStatus & 0x00000200)) ? "No overrun" : "Overrun") << "; ";
    ss << "COM3 buffer overrun flag: "
        << ((0 == (receiverStatus & 0x00000400)) ? "No overrun" : "Overrun") << "; ";
    ss << "USB buffer overrun flag: "
        << ((0 == (receiverStatus & 0x00000800)) ? "No overrun" : "Overrun") << "; ";
    ss << "RF1 AGC status: "
        << ((0 == (receiverStatus & 0x00008000)) ? "OK" : "Bad") << "; ";
    ss << "RF2 AGC status: "
        << ((0 == (receiverStatus & 0x00020000)) ? "OK" : "Bad") << "; ";
    ss << "Almanac flag/UTC known: "
        << ((0 == (receiverStatus & 0x00040000)) ? "Valid" : "Invalid") << "; ";
    ss << "Position solution flag: "
        << ((0 == (receiverStatus & 0x00080000)) ? "Valid" : "Invalid") << "; ";
    ss << "Position fixed flag: "
        << ((0 == (receiverStatus & 0x00100000)) ? "Not" : "fixed Fixed") << "; ";
    ss << "Clock steering status: "
        << ((0 == (receiverStatus & 0x00200000)) ? "Enabled" : "Disabled") << "; ";
    ss << "Clock model flag: "
        << ((0 == (receiverStatus & 0x00400000)) ? "Valid" : "Invalid") << "; ";
    ss << "OEMV card external oscillator flag: "
        << ((0 == (receiverStatus & 0x00800000)) ? "Disabled" : "Enabled") << "; ";
    ss << "Software resource: "
        << ((0 == (receiverStatus & 0x01000000)) ? "OK" : "Warning") << "; ";
    ss << "Auxiliary 3 status event flag: "
        << ((0 == (receiverStatus & 0x20000000)) ? "No event" : "Event") << "; ";
    ss << "Auxiliary 2 status event flag: "
        << ((0 == (receiverStatus & 0x40000000)) ? "No event" : "Event") << "; ";
    ss << "Auxiliary 1 status event flag: "
        << ((0 == (receiverStatus & 0x80000000)) ? "No event" : "Event");
    return ss.str();
}
}//namespace

#endif
