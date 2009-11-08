#include <iostream>
#include <sstream>
#include <gbxutilacfr/trivialtracer.h>
#include <gbxsmartbatteryacfr/gbxsmartbatteryacfr.h>

using namespace std;

string toString( const vector<string> &stringList )
{
    stringstream ss;
    for (unsigned int i=0; i<stringList.size(); ++i)
    {
        ss << stringList[i];
    }
    return ss.str();
    
}


int main( int argc, char **argv )
{
    int opt;
    std::string port = "/dev/ttyS0";
    int debug = 0;
    
    // Get some options from the command line
    while ((opt = getopt(argc, argv, "p:v")) != -1)
    {
        switch ( opt )
        {
            case 'p':
                port = optarg;
                break;
            case 'v':
                debug = 5;
                break;
            default:
                cout << "Usage: " << argv[0] << " [-p port] [-v(erbose)]" << endl
                     << "-p port\tPort the oceanserver battery system is connected to. E.g. /dev/ttyS0" << endl;
                return 1;
        }
    }
    
    int numRecords = 0;
    gbxutilacfr::TrivialTracer tracer( debug );    
    
    try 
    {
        gbxsmartbatteryacfr::BatteryHealthWarningConfig config;
        config.expectedNumBatteries = 2;
        config.numCyclesThreshhold = 300;
        config.chargeTempThreshhold = 40.0;
        config.dischargeTempThreshhold = 45.0;
        config.chargeWarnThreshhold = 10;
        config.chargeDeviationThreshold = 10;
        
        gbxsmartbatteryacfr::OceanServer oceanserver( port, tracer );
        
        while (true)    
        {   
            cout << "Reading record " << numRecords << endl;
            numRecords++;         
            
            gbxsmartbatteryacfr::OceanServerSystem data = oceanserver.getData();
            if ( data.isEmpty() )
            {
                cout << "Data was empty. No worries, keep trying to read." << endl;
                continue;
            }
            vector<string> shortWarning;
            vector<string> verboseWarning;
            const bool printRawRecord = true;
            bool haveWarnings = conductAllHealthChecks( data, config, shortWarning, verboseWarning, printRawRecord );
            
            if (haveWarnings)
            {
                cout << "Short warning messages: " << endl;
                cout << toString(shortWarning) << endl << endl;
                cout << "Verbose warning messages:" << endl;
                cout << toString(verboseWarning) << endl;
            }
            else
            {
                cout << "All systems normal." << endl;
            }
            
        }
    }
    catch ( gbxsmartbatteryacfr::HardwareReadingException &e )
    {
        cout << "ERROR(test): Caught a hardware reading exception: " 
                << e.what() << endl 
                << "This shouldn't happen!" << endl;
        return 1;
    }
    catch ( gbxutilacfr::Exception &e )
    {
        cout << "ERROR(test): Caught a gbxutilacfr::Exception: " 
             << e.what() << endl 
             << "This shouldn't happen!" << endl;
        return 1;
    }
    catch ( std::exception &e )
    {
        cout << "ERROR(test): Caught an unknown exception: " 
             << e.what() << endl
             << "This shouldn't happen!" << endl;
        return 1;
    }
    
    cout << "INFO(test): Successfully read " << numRecords << " records." << endl;
    
    return 0;
}

