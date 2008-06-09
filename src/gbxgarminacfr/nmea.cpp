/*
 * GearBox Project: Peer-Reviewed Open-Source Libraries for Robotics
 *               http://gearbox.sf.net/
 * Copyright (c) 2004-2008 Mathew Ridley, Alex Brooks, Alexei Makarenko, Tobias Kaupp
 *
 * This distribution is licensed to you under the terms described in
 * the LICENSE file included in this distribution.
 *
 */

#include <stdio.h>
#include <string>
#include <iostream>
#include <assert.h>
#include <gbxutilacfr/tokenise.h>

//////////////////////////////

// Ensure we have strnlen 
// eg. Solaris doesn't define strnlen in string.h, so define it here.
#if !HAVE_STRNLEN

#include <cstring>

// inline the fucker to guard against multiple inclusion, without the
// hassle of a special lib.
inline size_t strnlen(const char *s, size_t maxlen) 
{
    char *p;
    if (s == NULL) {
        return maxlen;
    }
    p = (char *)memchr(s, 0, maxlen);
    if (p == NULL) {
        return maxlen;
    }
    return ((p - s) + 1);
}
#endif

//////////////////////////////

#include "nmea.h"

using namespace std;
using namespace gbxgpsutilacfr;

const char NMEAStartOfSentence  = '$';
const char NMEAChecksumDelim    = '*';


//The blank constructor
NmeaMessage::NmeaMessage()
{
    init();
}

void NmeaMessage::init()
{
    haveSentence_ = false;
    haveTokens_   = false;
    haveCheckSum_ = false;
    checkSumOK_   = false;

    // Now clear the internal data store
    sentence_[0]   = 0;
    dataTokens_.clear();
}


NmeaMessage::NmeaMessage(const char *sentence, int testCheckSum)
{
    init();
    setSentence(sentence,testCheckSum);
}


//Load the data as requested and test the checksum if we are asked to.
void 
NmeaMessage::setSentence(const char *data, int AddOrTestCheckSum)
{
    init();
    
    strncpy(sentence_,data, MAX_SENTENCE_LEN);

    //terminate just in case, Note that we have a buffer which is
    //MAX_SENTENCE_LEN + 1 long!
    
    sentence_[MAX_SENTENCE_LEN] = '\0';
    haveSentence_             = true;

    switch ( AddOrTestCheckSum )
    {
        case TestChecksum:  { 
            // This is for Rx'd data that we need to test for correct reception
            // (internally it will also call addCheckSum())
            testChecksumOk(); 
            break;
        }
        case AddChecksum: {  
            // This is for Tx data that needs to checksummed before sending
            addCheckSum(); 
            checkSumOK_ = true; 
            break;
        }
        case DontTestOrAddChecksum: 
            break;
        default:
            assert( true && "unrecognized message option" );
    }
}

bool 
NmeaMessage::testChecksumOk()
{
    haveCheckSum_ = true;
    checkSumOK_   = false;

    //First save the checksum chars from the existing message
    char* ptr;
    char  chksum_HIB,chksum_LOB;
   
    //First save the existing two checksum chars from the message
    //These are straight after the '*' character
    ptr = strchr(sentence_, NMEAChecksumDelim);
    if ( !ptr ) {
        cout<<"device: no checksum delimiter"<<endl;
        return false;
    }
   
    //save the high and low bytes of the checksum
    //Make sure they are in upper case!
    chksum_HIB = toupper(*(++ptr));  
    chksum_LOB = toupper(*(ptr + 1));
   

    //invalidate the existing checksum
    *ptr = *(ptr+1) = 'x';
        
    //****NOTE** We leave the ptr pointing at the first chksum byte
       
    //Re-calculate our own copy of the checksum
    addCheckSum();

    //Now compare our saved version with our new ones
    if( (chksum_HIB == *ptr) && (chksum_LOB == *(ptr+1)) ) {
        //all looked good!
        checkSumOK_ = true;
        return true;
    }
   
    //failed the checksum!
    cout<<"device: '"<<chksum_HIB<<chksum_LOB   <<"' ("<<std::hex<<(unsigned int)chksum_HIB<<","<<(unsigned int)chksum_LOB<<std::dec<<") "
        <<"driver: '"<<*ptr<<*(ptr+1)<<"'"      <<"' ("<<std::hex<<(unsigned int)*ptr<<","<<(unsigned int)*(ptr+1)<<std::dec<<") "<<endl;
    return false;
}

// Add the checksum chars to an existing message
// NOTE: this assumes that there is allready space in the message for
// the checksum, and that the checksum delimiter is there
void 
NmeaMessage::addCheckSum()
{         
    assert( haveSentence_ && "calling addCheckSum() without a sentence" );
    
    haveCheckSum_ = true;

    //check that we have the '$' at the start
    if ( sentence_[0]!= NMEAStartOfSentence ) {
        throw NmeaException("cannot calculate checksum, missing leading '$'");
    }

    unsigned char chkRunning = 0;
    
    int loopCount;
    unsigned char nextChar;
    // we start from 1 to skip the leading '$'
    for( loopCount=1; loopCount<MAX_SENTENCE_LEN; ++loopCount ) 
    { 
        nextChar = static_cast<unsigned char>(sentence_[loopCount]);
    
        // no delimiter uh oh
        if( (nextChar=='\r') || (nextChar=='\n') || (nextChar=='\0') ) {
            throw NmeaException("cannot calculate checksum, missing final '*'");
        }
		
        // goodie we found it (the '*' is not included into the checksum)
        if ( nextChar==NMEAChecksumDelim ) {
            break;
        }
    
        // Keep the running XOR total
        chkRunning ^= nextChar;
    }
        
    //Put the byte values as upper case HEX back into the message
    sprintf( sentence_ + loopCount + 1,"%02X", chkRunning );
}

// Parse the data fields of our message...
void 
NmeaMessage::parseTokens()
{
    //We should not attempt to be parsing a message twice...
    assert( numDataTokens()==0 && "calling parseTokens() with tokens" );

    //Split the message at the commas
    //TODO cope with missing fields
    dataTokens_ = gbxutilacfr::tokenise(sentence_, ",");
    
    //Now discard the $ and the * from the first and last tokens...
    //TODO : - dataTokens_[0] = 

    //keep track of what we have done.
    haveTokens_ = true;
}
