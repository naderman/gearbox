#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/time.h>
#include <vector>
#include <string>

#include <gbxserialacfr/serial.h>

using namespace std;

static const unsigned int BUFFER_SIZE = 1024;
static const int BAUDRATE = 19200;
static const int TIMEOUT_SEC = 30;
static string DEVICE="/dev/ttyS3";

// ofstream* createFile()
// {
//     string filename="/home/mrsys/tmp/ocean/serialtest.txt";
//     ofstream *file = new ofstream( filename.c_str(), ios::app );
//     if ( !file->is_open() ) {
//         cout << "Could not create file " << filename << endl;
//         exit(1);
//     }
//     struct timeval now;
//     gettimeofday( &now, 0 );
//     (*file) << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << endl;
//     (*file) << "Start time (sec): " << now.tv_sec << endl << endl;

//     return file;
// }

bool isOceanServerSystem( const char* oceanServerString, gbxserialacfr::Serial &serial_ )
{
    vector<string> oceanServerStrings_;
    oceanServerStrings_.push_back(" S - Setup Controller");
    oceanServerStrings_.push_back(" B - Battery Status");
    oceanServerStrings_.push_back(" X - Host HEX");
    oceanServerStrings_.push_back(" H - Help");
    oceanServerStrings_.push_back(" www.ocean-server.com");
    
    for (unsigned int i=0; i<oceanServerStrings_.size(); i++)
    {
        // if the first 8 characters agree we are pretty sure we have an OceanServerSystem
        if (strncmp(oceanServerStrings_[i].c_str(),oceanServerString,8)==0) return true;
    }
    return false;
}

int main( int argc, char **argv )
{
    gbxserialacfr::Serial serial_( DEVICE, BAUDRATE, gbxserialacfr::Serial::Timeout(TIMEOUT_SEC,0) );
    
    const char menuMode = ' ';
    serial_.write(&menuMode, 1);
    
    const int maxTries = 40;
    int numTries=0;

    while(true)
    {
        cout << "INFO(oceanserverreader.cpp): Trying to read from serial port with timeout of " << TIMEOUT_SEC << "s" << endl;
        string serialData;
        int ret = serial_.readLine( serialData );
        if (ret<0)  {
            cout << "Connected to the wrong serial port. Timed out while trying to read a line." << endl;
        }
        if ( isOceanServerSystem( serialData.c_str(), serial_ ) ) {
            cout << "INFO(oceanserverreader.cpp): We are connected to an Oceanserver system. Good." << endl;
            break;
        }
        numTries++;
        cout << "Trying to find out whether this is an oceanserver system. Attempt number " << numTries << "/" << maxTries << "." << endl;
        if (numTries>=maxTries) {
            cout << "Connected to the wrong serial port. Didn't recognize any of the strings." << endl;
        }
    }

    // send an X
    serial_.flush();
    const char startReading = 'X';
    serial_.write(&startReading, 1);
    
    // ofstream* file = createFile();
    
    while(true)
    {
        std::string serialData;

        int ret = serial_.readLine( serialData );
        
        cout << "Ret: " << ret << " " << serialData << flush;
    }
    
    return 0;
}
