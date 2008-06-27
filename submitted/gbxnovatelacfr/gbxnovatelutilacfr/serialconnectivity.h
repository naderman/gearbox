/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Michael Moser
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */
namespace gbxserialacfr{
    class Serial;
}

// test connectivity to a [serial] device at a [baudrate];
// Assumes that you can figure out a [challenge] (command ...) to
// which the device will answer with a unique [ack] in [timeOutMsec] milliseconds.
// If [successThresh] or more [challenge]es are answered by an [ack]
//
// Limitations: amount of data expected before timeOutMsec should be
// reasonably small
//
// returns 0 for Success; -1 for failure
namespace gbxnovatelutilacfr{
bool testConnectivity(
        std::string &challenge,
        std::string &ack,
        gbxserialacfr::Serial& serial,
        int timeOutMsec,
        int numTry,
        int successThresh,
        int baudrate);
}
