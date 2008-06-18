#include <stdlib.h>
#include <unistd.h>

#include <gbxserialacfr/gbxserialacfr.h>
#include <string>
#include <vector>
#include <iostream>
#include <assert.h>

#include "serialconnectivity.h"

using namespace gbxserialacfr;

namespace gbxnovatelutilacfr{
// test connectivity to a [serial] device at a [baudrate];
// Assumes that you can figure out a [challenge] (e.g. a command) to
// which the device will answer with a unique [ack] in [timeOutMsec] milliseconds.
//
// If [successThresh] or more [challenge]es are answered by an [ack] we assume
// we got the correct baudrate.
//
// returns 0 for Success; -1 for failure
bool
testConnectivity(
        std::string &challenge,
        std::string &ack,
        gbxserialacfr::Serial& serial,
        int timeOutMsec,
        int numTry,
        int successThresh,
        int baudrate){
    std::cout << "Testing connectivity at " << baudrate << " bps: ";
    try{
        serial.setBaudRate(baudrate);
        serial.flush();
    }
    catch( SerialException &e ){
        std::cout <<"Caught SerialException: " << e.what()<<"\n";
        return false;
    }
    catch( std::exception &e ){
        std::cout <<"Caught std::exception: " << e.what()<<"\n";
        return false;
    }
    catch(...){
        std::cout <<"Caught unknown Exception :-(\n";
        return false;
    }

    int successCnt = 0;
    for(int i=0; i<numTry; i++){
        // send challenge
        serial.writeString(challenge.c_str());
        usleep(timeOutMsec*1000);

        //read response
        int available;
        available = serial.bytesAvailable();
        if(0<available){
            char *buf;
            buf = new char[available+1];
            int read = serial.read(buf, available);
            assert( read != -1 );

            //and look for an ACK
            buf[read] = '\0';
            std::string response(buf);
            size_t found = response.find(ack);
            if(std::string::npos != found){
                successCnt++;
            }
            delete []buf;
        }
    }
    std::cout << successCnt << "/"<< numTry << " ";
    if(successThresh <= successCnt){
        std::cout << "OK\n";
        return true;
    }else{
        std::cout << "Fail\n";
        return false;
    }
}
}//namespace
